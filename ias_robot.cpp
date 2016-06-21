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

#include "ias_robot.hpp"

IASSS_Robot::IASSS_Robot(std::string hostname, uint32_t port) : Robot(hostname, port) {
  std::cout << "Creating IAS-SS robot on host \"" << hostname << "\" and port " << port << "." << std::endl;
}

IASSS_Robot::~IASSS_Robot() {
  std::cout << "Destroying IAS-SS robot." << std::endl;
}

void IASSS_Robot::run() {
  _p_client->Read();
  
  if(_r_proxy->GetRange(1) < 1.0) {
    if(_r_proxy->GetRange(0) > 1.0)
      _p_proxy->SetSpeed(0.0f, PlayerCc::dtor(-45));
    else if(_r_proxy->GetRange(2) > 1.0)
      _p_proxy->SetSpeed(0.0f, PlayerCc::dtor(45));
    else
      _p_proxy->SetSpeed(0.0f, PlayerCc::dtor(180));
  } else {
    _p_proxy->SetSpeed(0.3f, 0.0f);
  }
  sleep(1);
}
