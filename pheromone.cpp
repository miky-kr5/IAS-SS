#include <iostream>
#include <cstdlib>
#include <pnglite.h>

#include "pheromone.hpp"

PheromoneMap::PheromoneMap(const char * file_name) {
  load_map(file_name);
  sem_init(&map_semaphore, 0, 1);
  tex_created = false;
  then = 0;
}

PheromoneMap::~PheromoneMap() {
  free(data);
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
  data = static_cast<unsigned char *>(malloc(tex.width * tex.height * tex.bpp));
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    tex_created = true;
  } sem_post(&map_semaphore);

  return handle;
}

void PheromoneMap::s_draw_point(float x, float y) {
  int _x = m_width * x;
  int _y = m_height - (m_height * y);

  sem_wait(&map_semaphore); {
    data[(_y * m_height) + _x] = 249;
  } sem_post(&map_semaphore);
}

void PheromoneMap::s_update() {
  clock_t now = clock();
  
  if(static_cast<float>(now - then) / CLOCKS_PER_SEC < 0.005) {
    return;
  }

  then = now;
  
  sem_wait(&map_semaphore); {
    for(int i = 1; i < m_height - 1; i++) {
      for(int j = 1; j < m_width - 1; j++) {
	if(data[(i * m_height) + j] > 10 && data[(i * m_height) + j] < 250){
	  data[(i * m_height) + j] -= 1;
	
	  if(data[((i - 1) * m_height) + (j - 1)] < 250)
	    data[((i - 1) * m_height) + (j - 1)] = data[(i * m_height) + j];
	
	  if(data[((i - 1) * m_height) + j] < 250)
	    data[((i - 1) * m_height) + j] = data[(i * m_height) + j];
	
	  if(data[((i - 1) * m_height) + (j + 1)] < 250)
	    data[((i - 1) * m_height) + (j + 1)] = data[(i * m_height) + j];
	
	  if(data[(i * m_height) + (j - 1)] < 250)
	    data[(i * m_height) + (j - 1)] = data[(i * m_height) + j];
	
	  if(data[(i * m_height) + (j + 1)] < 250)
	    data[(i * m_height) + (j + 1)] = data[(i * m_height) + j];
	
	  if(data[((i + 1) * m_height) + (j - 1)] < 250)
	    data[((i + 1) * m_height) + (j - 1)] = data[(i * m_height) + j];
	
	  if(data[((i + 1) * m_height) + j] < 250)
	    data[((i + 1) * m_height) + j] = data[(i * m_height) + j];
	
	  if(data[((i + 1) * m_height) + (j + 1)] < 250)
	    data[((i + 1) * m_height) + (j + 1)] = data[(i * m_height) + j];
	}
      }
    }
  } sem_post(&map_semaphore);
}
