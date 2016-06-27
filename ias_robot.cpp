/*************************************************************************************
 * Copyright (c) 2016, Miguel Angel Astor Romero                                     *
 * All rights reserved.                                                              *
 *                                                                                   *
 * Redistribution and use in source and binary forms, with or without                *
 * modification, are permitted provided that the following conditions are met:       *
 *                                                                                   *
 * 1. Redistributions of source code must retain the above copyright notice, this    *
 *    list of conditions and the following disclaimer.                               *
 * 2. Redistributions in binary form must reproduce the above copyright notice,      *
 *    this list of conditions and the following disclaimer in the documentation      *
 *    and/or other materials provided with the distribution.                         *
 *                                                                                   *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND   *
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED     *
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE            *
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR   *
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES    *
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;      *
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND       *
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT        *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS     *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                      *
 *************************************************************************************/

#include <limits>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>

#include "ias_robot.hpp"

static const float  TURN_DEG_PER_SEC = 40.0f;
static const float  METERS_PER_SEC   = 0.4f;
static const long   HALF_SECOND_USEC = 500000;
static const double MIN_DIST_M       = 1.5;
static const double CRIT_DIST_M      = 1.0;
static const float  MAP_SIZE         = 16.0f;
static const int    PHERO_AMOUNT     = 10;
static const float  PHERO_RADIUS     = 1.0;
static const float  SENSOR_RADIUS    = 2.0;

static inline float random_num() {
  return (((static_cast<float>(rand() % 256) / 256.0) - 0.5f) * 2.0f ) * PHERO_RADIUS;
}

IASSS_Robot::IASSS_Robot(std::string hostname, uint32_t port, PheromoneMap * phero_map) : Robot(hostname, port) {
  srand(port + time(NULL));
  _phero_map = phero_map;
  log("Creating IAS-SS robot");
}

IASSS_Robot::~IASSS_Robot() {
  log("Destroying IAS-SS robot");
}

void IASSS_Robot::run() {
  float x, y;
  int    rv;
  long   then, now, delta, wait;
  struct timeval tv;
  double dist   = std::numeric_limits<double>::infinity();

  _p_client->Read();

  rv = gettimeofday(&tv, NULL);
  then = tv.tv_usec;

  /******************************************************************************
   * WALL AVOIDANCE START                                                       *
   ******************************************************************************/
  // Check if there is something in front of the robot.
  for(int i = 96; i < 126; i++)
    dist = _r_proxy->GetRange(i) < dist ? _r_proxy->GetRange(i) : dist;

  if(dist < MIN_DIST_M && dist > CRIT_DIST_M) {
    avoid_wall(METERS_PER_SEC, TURN_DEG_PER_SEC);
  } else if(dist < CRIT_DIST_M) {
    avoid_wall(0.0f, TURN_DEG_PER_SEC);
  } else
    _p_proxy->SetSpeed(METERS_PER_SEC, 0.0f);
  /******************************************************************************
   * WALL AVOIDANCE END                                                         *
   ******************************************************************************/

  x = (_p_proxy->GetXPos() + (MAP_SIZE / 2)) / MAP_SIZE;
  y = (_p_proxy->GetYPos() + (MAP_SIZE / 2)) / MAP_SIZE;
  _phero_map->s_sample(&_phero_sensor, x, y, _p_proxy->GetYaw(), SENSOR_RADIUS / MAP_SIZE);
  
  deposit_pheromone();

  rv = gettimeofday(&tv, NULL);
  now = tv.tv_usec;
  delta = now - then;
  
  // Sleep for a bit before finishing this control iteration.
  wait = rv == 0 ? HALF_SECOND_USEC - delta : HALF_SECOND_USEC;
  usleep(wait);
}

void IASSS_Robot::avoid_wall(float front_speed, float turn_speed) {
  double dist_l = 0.0;
  double dist_r = 0.0;

  for(unsigned int i = 0; i < 96; i++)
    dist_r += _r_proxy->GetRange(i);
  dist_r /= 96;

  for(unsigned int i = 126; i < _r_proxy->GetRangeCount(); i++)
    dist_l += _r_proxy->GetRange(i);
  dist_l /= (_r_proxy->GetRangeCount() - 126);

  if(dist_r >= dist_l)
    _p_proxy->SetSpeed(front_speed, PlayerCc::dtor(-turn_speed));
  else
    _p_proxy->SetSpeed(front_speed, PlayerCc::dtor(turn_speed));
}

void IASSS_Robot::deposit_pheromone() {
  float x = _p_proxy->GetXPos();
  float y = _p_proxy->GetYPos();
  float px, py;

  if(_phero_map != NULL) {
    for(int i = 0; i < PHERO_AMOUNT; i++) {
      px = random_num() + x;
      py = random_num() + y;
      if(fabs(px) < (MAP_SIZE / 2) && fabs(py) < (MAP_SIZE / 2)) {
	px = (px + (MAP_SIZE / 2)) / MAP_SIZE;
	py = (py + (MAP_SIZE / 2)) / MAP_SIZE;
	if(!_phero_map->s_deposit_pheromone(px, py)) {
	  i--;
	  continue;
	}
      } else {
	i--;
	continue;
      }
    }
  }
}
