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

#ifndef IAS_ROBOT_HPP
#define IAS_ROBOT_HPP

#include "robot.hpp"
#include "pheromone.hpp"

namespace ias_ss {
  /**
   * Concrete robot that implements the IAS-SS architecture as defined in:
   *
   *   1) R. Calvo et al. "Inverse ACO for Exploration and Surveillance in
   *      Unknown Environments", The Third International Conference on Advanced
   *      Cognitive Technologies and Applications, Rome, Italy 2011.
   *
   *   2) R. Calvo et al. "A Distributed, Bio-Inspired Coordination Strategy
   *      for Multiple Agent Systems Applied to Surveillance Tasks in Unknown
   *      Environments", Proc. of the IEEE IJCNN, San Jose, USA, 2011.
   */
  class IASSS_Robot : Robot {
  public:
    IASSS_Robot(std::string hostname, uint32_t port, PheromoneMap * phero_map);
    virtual ~IASSS_Robot();
    virtual void run();

  private:
    PheromoneMap * _phero_map;
    phero_sensor_t _phero_sensor;
  
    void avoid_wall(float front_speed, float turn_speed);
    void deposit_pheromone(float x, float y);
    float brss();
  };
}

#endif
