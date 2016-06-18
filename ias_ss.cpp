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

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <csignal>

#include "robot.hpp"

const std::string HOST_NAME = "localhost";
const uint32_t    PORT = 6666;
const uint32_t    NUM_ROBOTS = 4;

static bool done = false;

extern "C" {
  void handler(int signal) {
    done = true;
  }
  
  void * robot_thread(void * arg) {
    IASSS_Robot * robot = static_cast<IASSS_Robot *>(arg);

    std::cout << "Running robot thread." << std::endl;

    while(!done)
      robot->run();

    return NULL;
  }
}

int main(int argc, char **argv) {
  pthread_t                  robot_threads[NUM_ROBOTS];
  std::vector<IASSS_Robot *> robots;

  signal(SIGINT, handler);
  
  try {
    // Initialize the robot objects and threads.
    for(uint32_t i = 0; i < NUM_ROBOTS; ++i) {
      robots.push_back(new IASSS_Robot(HOST_NAME, PORT + i));

      if(pthread_create(&robot_threads[i], NULL, robot_thread, static_cast<void *>(robots[i])) != 0) {
	perror("Could not create robot thread");
	return EXIT_FAILURE;
      }
    }

    // Wait for all the robots to finish.
    for(uint32_t i = 0; i < NUM_ROBOTS; ++i) {
      if(pthread_join(robot_threads[i], NULL) != 0) {
	perror("Could not join robot thread");
	return EXIT_FAILURE;
      }
      delete robots[i];
    }
    robots.clear();

  } catch (PlayerCc::PlayerError & e) {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
