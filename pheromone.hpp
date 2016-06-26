#ifndef PHEROMONE_HPP
#define PHEROMONE_HPP

#include <ctime>
#include <semaphore.h>
#include <GL/gl.h>

class PheromoneMap {
public:
  PheromoneMap(const char * file_name);
  ~PheromoneMap();

  GLuint s_build_texture();
  void s_draw_point(float x, float y);
  void s_update();
  
private:
  unsigned char * data;
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
