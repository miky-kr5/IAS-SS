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

#ifndef PHEROMONE_HPP
#define PHEROMONE_HPP

#include <cstring>
#include <ctime>
#include <semaphore.h>
#include <GL/gl.h>

extern const unsigned char MAX_PHERO_INTENSITY;
extern const unsigned char MIN_PHERO_INTENSITY;

const unsigned int NUM_PHERO_SAMPLES = 180;

typedef struct PHERO_SENSOR {
  float        samples[NUM_PHERO_SAMPLES];
  unsigned int sample_amnt[NUM_PHERO_SAMPLES];
  float        probs[NUM_PHERO_SAMPLES];

  PHERO_SENSOR() {
    reset();
  }

  void reset() {
    memset(sample_amnt, 0, sizeof(unsigned int) * NUM_PHERO_SAMPLES);
    for(unsigned int i = 0; i < NUM_PHERO_SAMPLES; i++) {
      samples[i] = 0.0f;
      probs[i]   = 0.0f;
    }
  }
  
  float operator[](unsigned int index) {
    if(index >= NUM_PHERO_SAMPLES)
      return -1.0f;
    else
      return samples[index];
  }
} phero_sensor_t;

class PheromoneMap {
public:
  PheromoneMap(const char * file_name);
  ~PheromoneMap();

  GLuint s_build_texture();
  void s_deposit_pheromone(float x, float y, float yaw, float radius);
  void s_evaporate();
  void s_sample(phero_sensor_t * sensor, float x, float y, float yaw, float radius);
  
private:
  int *           data;
  unsigned char * tex_data;
  unsigned        m_width;
  unsigned        m_height;
  unsigned char   m_bpp;
  sem_t           map_semaphore;
  bool            tex_created;
  GLuint          handle;
  clock_t         then;

  void load_map(const char * file_name);
};

#endif
