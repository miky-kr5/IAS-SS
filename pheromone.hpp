#ifndef PHEROMONE_HPP
#define PHEROMONE_HPP

#include <GL/gl.h>
#include <semaphore.h>

class PheromoneMap {
public:
  PheromoneMap(const char * file_name);
  ~PheromoneMap();

  GLuint s_build_texture();
  
private:
  unsigned char * data;
  unsigned        m_width;
  unsigned        m_height;
  unsigned char   m_bpp;
  sem_t map_semaphore;
  bool tex_created;
  GLuint handle;

  void load_map(const char * file_name);
};

#endif
