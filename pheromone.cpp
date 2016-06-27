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

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <pnglite.h>
#include <glm/glm.hpp>

#include "pheromone.hpp"

#define MAP_POS(X, Y) (data[((X) * m_height) + (Y)])

static const float         EVAPORATION_RATE    = 0.1f;
const        unsigned char MAX_PHERO_INTENSITY = 250;
const        unsigned char MIN_PHERO_INTENSITY = 1;

PheromoneMap::PheromoneMap(const char * file_name) {
  load_map(file_name);
  sem_init(&map_semaphore, 0, 1);
  tex_created = false;
  then = 0;
}

PheromoneMap::~PheromoneMap() {
  delete data;
  sem_destroy(&map_semaphore);

  if(tex_created) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, handle);
    glDeleteTextures(1, &handle);
  }
}

void PheromoneMap::load_map(const char * file_name) {
  png_t tex;

  png_init(0, 0);
  png_open_file_read(&tex, file_name);
  data = new unsigned char[tex.width * tex.height * tex.bpp];
  png_get_data(&tex, data);

  std::cout << "Loaded map \"" << file_name << "\" :: " << tex.width << "x" << tex.height << "x" << (int)tex.bpp << std::endl;
  m_width  = tex.width;
  m_height = tex.height;
  m_bpp    = tex.bpp;
  
  png_close_file(&tex);
}

GLuint PheromoneMap::s_build_texture() {
  sem_wait(&map_semaphore); {
    if(tex_created) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, handle);
      glDeleteTextures(1, &handle);
    }
    
    glGenTextures(1, &handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_width, m_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    tex_created = true;
  } sem_post(&map_semaphore);

  return handle;
}

bool PheromoneMap::s_deposit_pheromone(float x, float y) {
  bool valid = false;
  unsigned int  _x    = m_width * x;
  unsigned int  _y    = m_height - (m_height * y);

  if((y > 1.0 || y < 0.0) || (x > 1.0 || x < 0.0))
    return false;
  
  sem_wait(&map_semaphore); {
    if((_y >= m_height || _y < 0) || (_x >= m_width || _x < 0))
      return false;
    
    if(MAP_POS(_y, _x) <= MAX_PHERO_INTENSITY) {
      MAP_POS(_y, _x) = MAX_PHERO_INTENSITY;
      valid = true;
    }
  } sem_post(&map_semaphore);

  return valid;
}

void PheromoneMap::s_evaporate() {
  unsigned char p_eva;

  clock_t now = clock();
  if(static_cast<float>(now - then) / CLOCKS_PER_SEC < 0.05) {
    return;
  }
  then = now;
  
  sem_wait(&map_semaphore); {
    for(unsigned i = 1; i < m_height - 1; i++) {
      for(unsigned j = 1; j < m_width - 1; j++) {
	if(MAP_POS(i, j) >= MIN_PHERO_INTENSITY && MAP_POS(i, j) <= MAX_PHERO_INTENSITY) {
	  p_eva = MAP_POS(i, j) * EVAPORATION_RATE;
	  MAP_POS(i, j) -= p_eva;
	}
      }
    }
  } sem_post(&map_semaphore);
}

void PheromoneMap::s_sample(phero_sensor_t * sensor, float x, float y, float yaw, float radius) {
  float       _x = x;
  float       _y = y;
  float     _r = radius;
  float     dist;
  float     cos_theta;
  glm::vec2 v;
  glm::vec2 vp;

  if(sensor == NULL)
    return;
  else {
    v  = glm::vec2(_r * cos(yaw), _r * sin(yaw)) - glm::vec2(0.0, 0.0);
    v  = glm::normalize(v);
    
    sem_wait(&map_semaphore); {
      for(unsigned i = 1; i < m_height - 1; i++) {
	for(unsigned j = 1; j < m_width - 1; j++) {
	  vp        = glm::vec2(j/float(m_width), 1.0f - (i/float(m_height))) - glm::vec2(_x, _y);
	  dist      = glm::length(vp);
	  vp        = glm::normalize(vp);
	  cos_theta = glm::dot(vp, v);

	  if(cos_theta > 0.0f && dist <= _r) {
	    if(MAP_POS(i, j) >= MIN_PHERO_INTENSITY && MAP_POS(i, j) <= MAX_PHERO_INTENSITY) {
	      MAP_POS(i, j) = MAX_PHERO_INTENSITY;
	    }
	  } else
	    continue;
	}
      }
    } sem_post(&map_semaphore);
  }
}
