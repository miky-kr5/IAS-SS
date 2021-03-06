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

#include "gui.hpp"
#include "ias_robot.hpp"

const char * TITLE = "Pheromone map";
const char * S_TITLE = "Pheromone sensor";
const int W = 512;
const int H = 512;
const uint32_t PORT = PlayerCc::PLAYER_PORTNUM + 1;
const uint32_t NUM_ROBOTS = 4;

static bool                done      = false;
static Fl_Window *         window    = NULL;
static Fl_Window *         sWindow    = NULL;
static gui::GlGui *        glWindow  = NULL;
static gui::GlSensorGui *  glSWindow = NULL;
static ias_ss::PheromoneMap *      phero_map = NULL;

extern "C" void handler(int signal) {
  done = true;
}
  
extern "C" void * robot_thread(void * arg) {
  ias_ss::IASSS_Robot * robot = static_cast<ias_ss::IASSS_Robot *>(arg);

  std::cout << "Running robot thread." << std::endl;

  while(!done) {
    robot->run();
    pthread_testcancel();
  }

  return NULL;
}

void create_gui(int argc, char **argv) {
  window = new Fl_Window(20, 40, W, H, TITLE);
  glWindow = new gui::GlGui(window, 0, 0, W, H, TITLE, phero_map);
  window->end();
  window->show(argc, argv);
  window->make_current();

  sWindow = new Fl_Window(20, 40, W, H, S_TITLE);
  glSWindow = new gui::GlSensorGui(sWindow, 0, 0, W, H, S_TITLE, phero_map);
  sWindow->end();
  sWindow->show(argc, argv);
  sWindow->make_current();
}

int main(int argc, char **argv) {
  pthread_t                  robot_threads[NUM_ROBOTS];
  std::vector<ias_ss::IASSS_Robot *> robots;

  signal(SIGINT, handler);

  try {
    phero_map = new ias_ss::PheromoneMap(argc > 1 ? argv[1] : "maps/cave_mask.png");

    // Initialize the robot objects and threads.
    for(uint32_t i = 0; i < NUM_ROBOTS; ++i) {
      robots.push_back(new ias_ss::IASSS_Robot(PlayerCc::PLAYER_HOSTNAME, PORT + i, phero_map));

      if(pthread_create(&robot_threads[i], NULL, robot_thread, static_cast<void *>(robots[i])) != 0) {
	perror("Could not create robot thread");
	return EXIT_FAILURE;
      }
    }

    create_gui(argc, argv);
    Fl::run();
    
    // Finish all the robots.
    for(uint32_t i = 0; i < NUM_ROBOTS; ++i)
      pthread_cancel(robot_threads[i]);

    for(uint32_t i = 0; i < NUM_ROBOTS; ++i) {
      if(pthread_join(robot_threads[i], NULL) != 0) {
	perror("Could not join robot thread");
	return EXIT_FAILURE;
      }
      delete robots[i];
    }
    robots.clear();

    delete phero_map;
    
  } catch (PlayerCc::PlayerError & e) {
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
