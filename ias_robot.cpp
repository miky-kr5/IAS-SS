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

#include <map>
#include <limits>
#include <cstdlib>
#include <cmath>
#include <sys/time.h>
#ifndef NDEBUG
#include <sstream>
#endif

#include "ias_robot.hpp"

//#define PROB_MODEL_1

namespace ias_ss {
  static const float  TURN_DEG_PER_SEC = 90.0f;
  static const float  METERS_PER_SEC   = 0.4f;
  static const long   HALF_SECOND_USEC = 500000;
  static const double MIN_DIST_M       = 1.0;
  static const double CRIT_DIST_M      = 0.7;
  static const float  MAP_SIZE         = 16.0f;
  static const int    PHERO_AMOUNT     = 30;
  static const float  PHERO_RADIUS     = 1.4f;
  static const float  SENSOR_RADIUS    = 2.0f;
  static const float  U_RATIO          = 0.3f;
  static const float  V_RATIO          = 0.1f;
  static const float  SMOOTHING_COEFF  = 0.3f;

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
    float x, y, steer = 0.0f;
    int    rv;
    long   then, now, delta, wait;
    struct timeval tv;
    double dist  = std::numeric_limits<double>::infinity();

    _p_client->Read();

    rv = gettimeofday(&tv, NULL);
    then = tv.tv_usec;

    x = (_p_proxy->GetXPos() + (MAP_SIZE / 2)) / MAP_SIZE;
    y = (_p_proxy->GetYPos() + (MAP_SIZE / 2)) / MAP_SIZE;
    _phero_map->s_sample(&_phero_sensor, x, y, _p_proxy->GetYaw(), SENSOR_RADIUS / MAP_SIZE);
  
    steer += SMOOTHING_COEFF * brss();

    deposit_pheromone(x, y);
  
    /******************************************************************************
     * WALL AVOIDANCE START                                                       *
     ******************************************************************************/
    // Check if there is something in front of the robot.
    for(int i = 89; i < 134; i++)
      dist = _r_proxy->GetRange(i) < dist ? _r_proxy->GetRange(i) : dist;

    if(dist < MIN_DIST_M && dist > CRIT_DIST_M) {
      avoid_wall(METERS_PER_SEC, TURN_DEG_PER_SEC);
    } else if(dist <= CRIT_DIST_M) {
      avoid_wall(0.0f, TURN_DEG_PER_SEC);
    } else
      _p_proxy->SetSpeed(METERS_PER_SEC, steer);
    /******************************************************************************
     * WALL AVOIDANCE END                                                         *
     ******************************************************************************/

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

  void IASSS_Robot::deposit_pheromone(float x, float y) {
    for(int i = 0; i < PHERO_AMOUNT; i++)
      _phero_map->s_deposit_pheromone(x, y, _p_proxy->GetYaw(), PHERO_RADIUS / MAP_SIZE);
  }

  float IASSS_Robot::brss() {
    std::map<int, float> U, V;
    unsigned int         i_min, i_max;
    float                min, sample, prob, max, sum_uv = 0.0f, steer;
#ifndef NDEBUG
    std::ostringstream   oss;
#endif

    while(U.size() < (U_RATIO * NUM_PHERO_SAMPLES)) {
      min = std::numeric_limits<double>::max();
      i_min = 0;
      for(unsigned int i = NUM_PHERO_SAMPLES / 2; i < NUM_PHERO_SAMPLES; i++) {
	sample = _phero_sensor[i];
	if(U.find(i) == U.end()) {
	  if(sample < min) {
	    min = sample;
	    i_min = i;
	  }
	}
      }
      for(unsigned int i = NUM_PHERO_SAMPLES / 2; i > 0; i--) {
	sample = _phero_sensor[i];
	if(U.find(i) == U.end()) {
	  if(sample < min) {
	    min = sample;
	    i_min = i;
	  }
	}
      }
      U[i_min] = min;
    }

    while(V.size() < (V_RATIO * NUM_PHERO_SAMPLES)) {
      for(unsigned int i = NUM_PHERO_SAMPLES / 2; i < NUM_PHERO_SAMPLES; i++) {
	if(U.find(i) == U.end() && V.find(i) == V.end()) {
	  prob = rand() % 100;
	  if(prob < 15)
	    V[i] = _phero_sensor[i];
	}
      }
      for(unsigned int i = NUM_PHERO_SAMPLES / 2; i > 0; i--) {
	if(U.find(i) == U.end() && V.find(i) == V.end()) {
	  prob = rand() % 100;
	  if(prob < 15)
	    V[i] = _phero_sensor[i];
	}
      }
    }

    for(std::map<int, float>::iterator it = U.begin(); it != U.end(); ++it) {
#ifdef PROB_MODEL_1
      sum_uv += it->second;
#else
      sum_uv += 1.0f - it->second;
#endif
    }

    for(std::map<int, float>::iterator it = V.begin(); it != V.end(); ++it) {
#ifdef PROB_MODEL_1
      sum_uv += it->second;
#else
      sum_uv += 1.0f - it->second;
#endif
    }

    U.clear();
    V.clear();

    for(unsigned int i = 0; i < NUM_PHERO_SAMPLES; i++) {
#ifdef PROB_MODEL_1
      _phero_sensor.probs[i] = 1.0f / (_phero_sensor[i] / sum_uv);
#else
      _phero_sensor.probs[i] = (1.0f - _phero_sensor[i]) / (sum_uv);
#endif
    }

    max = std::numeric_limits<double>::min();
    i_max = 0;
    for(unsigned int i = NUM_PHERO_SAMPLES / 2; i < NUM_PHERO_SAMPLES; i++) {
      if(_phero_sensor.probs[i] > max) {
	max = _phero_sensor.probs[i];
	i_max = i;
      }
    }
    for(unsigned int i = NUM_PHERO_SAMPLES / 2; i > 0; i--) {
      if(_phero_sensor.probs[i] > max) {
	max = _phero_sensor.probs[i];
	i_max = i;
      }
    }

    steer = (NUM_PHERO_SAMPLES / 2.0f) - i_max;

#ifndef NDEBUG
    oss << "samples: " << std::endl;
    for(unsigned int i = 0; i < NUM_PHERO_SAMPLES; i++)
      oss << "\tSAMPLE[" << i << "]: " << _phero_sensor[i] << " - " << _phero_sensor.sample_amnt[i] << " - " << _phero_sensor.probs[i] << std::endl;
    oss << "\ti_max: " << i_max << " | Steer: " << steer;
  
    log(oss.str());
#endif

    return steer;
  }
}
