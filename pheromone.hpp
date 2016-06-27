#ifndef PHEROMONE_HPP
#define PHEROMONE_HPP

#include <ctime>
#include <semaphore.h>
#include <GL/gl.h>

extern const unsigned char MAX_PHERO_INTENSITY;
extern const unsigned char MIN_PHERO_INTENSITY;

class PheromoneMap {
public:
  PheromoneMap(const char * file_name);
  ~PheromoneMap();

  GLuint s_build_texture();
  bool s_deposit_pheromone(float x, float y);
  void s_evaporate();
  
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
