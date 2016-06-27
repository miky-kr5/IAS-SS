#include <iostream>
#include <cstdlib>
#include <pnglite.h>

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
  int  _x = m_width * x;
  int  _y = m_height - (m_height * y);

  sem_wait(&map_semaphore); {
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
	if(MAP_POS(i, j) >= MIN_PHERO_INTENSITY && MAP_POS(i, j) <= MAX_PHERO_INTENSITY){
	  p_eva = MAP_POS(i, j) * EVAPORATION_RATE;
	  MAP_POS(i, j) -= p_eva;
	}
      }
    }
  } sem_post(&map_semaphore);
}
