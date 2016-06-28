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
#include <cassert>
#include <pnglite.h>
#include <glm/glm.hpp>
#include <libplayerc++/playerc++.h>

#include "pheromone.hpp"

#define MAP_POS(X, Y) (data[((X) * m_height) + (Y)])

static const unsigned int  MAX_ITERS           = 1000;
static const float         EVAPORATION_RATE    = 0.1f;
const        unsigned char MAX_PHERO_INTENSITY = 250;
const        unsigned char MIN_PHERO_INTENSITY = 0;

static inline int sign(float f) {
  return (f < 0.0f) ? -1 : ((f > 0.0f) ? 1 : 0);
}

static inline int side(glm::vec3 line, glm::vec3 v) {
  return sign(glm::cross(line, v).z);
}

static inline float random_n(float r) {
  return (((static_cast<float>(rand() % 256) / 256.0) - 0.5f) * 2.0f ) * r;
}

PheromoneMap::PheromoneMap(const char * file_name) {
  load_map(file_name);
  sem_init(&map_semaphore, 0, 1);
  tex_created = false;
  then = 0;
}

PheromoneMap::~PheromoneMap() {
  delete data;
  delete tex_data;
  sem_destroy(&map_semaphore);

  if(tex_created) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, handle);
    glDeleteTextures(1, &handle);
  }
}

void PheromoneMap::load_map(const char * file_name) {
  unsigned char * _data;
  png_t tex;

  png_init(0, 0);
  png_open_file_read(&tex, file_name);
  _data = new unsigned char[tex.width * tex.height * tex.bpp];
  png_get_data(&tex, _data);

  std::cout << "Loaded map \"" << file_name << "\" :: " << tex.width << "x" << tex.height << "x" << (int)tex.bpp << std::endl;
  m_width  = tex.width;
  m_height = tex.height;
  m_bpp    = tex.bpp;
  
  png_close_file(&tex);

  data = new int[tex.width * tex.height * tex.bpp];
  tex_data = new unsigned char[tex.width * tex.height * tex.bpp];
  for(unsigned int i = 0; i < (tex.width * tex.height * tex.bpp); i++) {
    data[i] = _data[i];
  }
  delete _data;
}

GLuint PheromoneMap::s_build_texture() {
  sem_wait(&map_semaphore); {
    if(tex_created) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, handle);
      glDeleteTextures(1, &handle);
    }

    for(unsigned int i = 0; i < (m_width * m_height * m_bpp); i++) {
      tex_data[i] = static_cast<unsigned char>(data[i]);
    }

    glGenTextures(1, &handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, m_width, m_height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, tex_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    tex_created = true;
  } sem_post(&map_semaphore);

  return handle;
}

void PheromoneMap::s_deposit_pheromone(float x, float y, float yaw, float radius) {
  unsigned int iters = 0, _x, _y;
  bool         valid = false;
  float        dist, cos_theta, r_x, r_y;
  glm::vec2    v, vp;

  v  = glm::vec2(radius * cos(yaw), radius * sin(yaw)) - glm::vec2(0.0, 0.0);
  v  = glm::normalize(v);

  do {
    do {
      do {
	do {
	  r_x  = random_n(radius) + x;
	  r_y  = random_n(radius) + y;
	} while((r_x <= 0.0f && r_y <= 0.0f) || (r_x >= 1.0f && r_y >= 1.0f));
	vp        = glm::vec2(r_x, r_y) - glm::vec2(x, y);
	dist      = glm::length(vp);
      } while(dist > radius);
      vp        = glm::normalize(vp);
      cos_theta = glm::dot(vp, v);
    } while(cos_theta > 0.0f);
    _x = m_width * r_x;
    _y = m_height - (m_height * r_y);
    _x = _x < 0 ? _x = 0 : (_x >= m_width ? m_width - 1 : _x);
    _y = _y < 0 ? _y = 0 : (_y >= m_height ? m_height - 1 : _y);
    sem_wait(&map_semaphore); {
      if(MAP_POS(_y, _x) <= MAX_PHERO_INTENSITY) {
	MAP_POS(_y, _x) += rand() % MAX_PHERO_INTENSITY;
	MAP_POS(_y, _x) = MAP_POS(_y, _x) > MAX_PHERO_INTENSITY ? MAX_PHERO_INTENSITY : MAP_POS(_y, _x);
	valid = true;
      }
    } sem_post(&map_semaphore);
    iters++;
    if(iters > MAX_ITERS)
      break;
  } while(!valid);
}

void PheromoneMap::s_evaporate() {
  unsigned char p_eva;

  clock_t now = clock();
  if(static_cast<float>(now - then) / CLOCKS_PER_SEC < 0.09) {
    return;
  }
  then = now;
  
  sem_wait(&map_semaphore); {
    for(unsigned i = 0; i < m_height; i++) {
      for(unsigned j = 0; j < m_width; j++) {
	if(MAP_POS(i, j) <= MAX_PHERO_INTENSITY) {
	  p_eva = MAP_POS(i, j) * EVAPORATION_RATE;
	  MAP_POS(i, j) -= p_eva;
	  MAP_POS(i, j) = MAP_POS(i, j) < 0 ? 0 : MAP_POS(i, j);
	}
      }
    }
  } sem_post(&map_semaphore);
}

void PheromoneMap::s_sample(phero_sensor_t * sensor, float x, float y, float yaw, float radius) {
  unsigned int index;
  float        dist;
  float        cos_theta;
  float        ang;
  glm::vec2    v, vp;
  glm::vec3    line, v3d;

  if(sensor == NULL)
    return;
  else {
    sensor->reset();

    v  = glm::vec2(radius * cos(yaw), radius * sin(yaw)) - glm::vec2(0.0, 0.0);
    v  = glm::normalize(v);
    
    sem_wait(&map_semaphore); {
      for(unsigned i = 0; i < m_height; i++) {
	for(unsigned j = 0; j < m_width; j++) {
	  vp        = glm::vec2(j/float(m_width), 1.0f - (i/float(m_height))) - glm::vec2(x, y);
	  dist      = glm::length(vp);
	  vp        = glm::normalize(vp);
	  cos_theta = glm::dot(vp, v);

	  if(cos_theta > 0.0f && dist <= radius) {
	    cos_theta = cos_theta > 1.0f ? 1.0f : cos_theta;
	    ang  = PlayerCc::rtod(acos(cos_theta));
	    line = glm::vec3(v.x, v.y, 0.0f);
	    v3d  = glm::vec3(vp.x, vp.y, 0.0f);
	    if(side(line, v3d) > 0) {
	      index = static_cast<unsigned int>((NUM_PHERO_SAMPLES / 2.0f) - ang);
	      index = index >= NUM_PHERO_SAMPLES ? NUM_PHERO_SAMPLES - 1 : index;
	      if(MAP_POS(i, j) > sensor->samples[index]) {
		sensor->samples[index] += MAP_POS(i, j);
		sensor->sample_amnt[index] += 1;
	      }
	    } else {
	      index = static_cast<unsigned int>((NUM_PHERO_SAMPLES / 2.0f) + ang);
	      index = index >= NUM_PHERO_SAMPLES ? NUM_PHERO_SAMPLES - 1 : index;
	      if(MAP_POS(i, j) > sensor->samples[index]) {
		sensor->samples[index] += MAP_POS(i, j);
		sensor->sample_amnt[index] += 1;
	      }
	    }
	  } else
	    continue;
	}
      }
    } sem_post(&map_semaphore);

    for(unsigned int i = 0; i < NUM_PHERO_SAMPLES; i++) {
      sensor->samples[i] = (sensor->sample_amnt[i] > 0) ? (sensor->samples[i] / sensor->sample_amnt[i]) : 0.0f;
    }
  }
}
