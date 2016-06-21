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
#include <sys/time.h>

#include "ias_robot.hpp"

static const long   HALF_SECOND_USEC = 500000;
static const double MIN_DIST_M       = 1.5;

IASSS_Robot::IASSS_Robot(std::string hostname, uint32_t port) : Robot(hostname, port) {
  std::cout << "Creating IAS-SS robot on host \"" << hostname << "\" and port " << port << "." << std::endl;
}

IASSS_Robot::~IASSS_Robot() {
  std::cout << "Destroying IAS-SS robot on " << _host_name << ":" << _port << std::endl;
}

void IASSS_Robot::run() {
  int    rv;
  long   then, now, delta, wait;
  struct timeval tv;
  double dist   = std::numeric_limits<double>::infinity();
  double dist_l = 0.0;
  double dist_r = 0.0;

  _p_client->Read();
  rv = gettimeofday(&tv, NULL);
  then = tv.tv_usec;
  /******************************************************************************
   * WALL AVOIDANCE START                                                       *
   ******************************************************************************/
  // Check if there is something in front of the robot.
  for(int i = 96; i < 126; i++)
    dist = _r_proxy->GetRange(i) < dist ? _r_proxy->GetRange(i) : dist;

  if(dist < MIN_DIST_M) {
    for(unsigned int i = 0; i < 96; i++)
      dist_r += _r_proxy->GetRange(i);
    dist_r /= 96;

    for(unsigned int i = 126; i < _r_proxy->GetRangeCount(); i++)
      dist_l += _r_proxy->GetRange(i);
    dist_l /= (_r_proxy->GetRangeCount() - 126);

    if(dist_r >= dist_l)
      _p_proxy->SetSpeed(0.0f, PlayerCc::dtor(-20));
    else
      _p_proxy->SetSpeed(0.0f, PlayerCc::dtor(20));
  } else
    _p_proxy->SetSpeed(0.4f, 0.0f);
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
