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
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <pnglite.h>

#include "ogl.hpp"
#include "GLSLProgram.hpp"

namespace ogl
{ 
  // Variables
  static CGLSLProgram m_program;
  static ias_ss::PheromoneMap * m_phero_map = NULL;
  static GLuint m_textureHandle;
  static GLuint m_sensorTextureHandle;
  static GLuint m_colorMapHandle;

  // Quad definition
  static glm::vec4 vec_points[6];
  static glm::vec2 vec_tex_coords[6];
  
  static inline void quad() {
    vec_tex_coords[0] = glm::vec2( 0.0f,  1.0f); vec_points[0] = glm::vec4( -0.5f, -0.5f, 0.0f, 1.0f );
    vec_tex_coords[1] = glm::vec2( 0.0f,  0.0f); vec_points[1] = glm::vec4( -0.5f,  0.5f, 0.0f, 1.0f );
    vec_tex_coords[2] = glm::vec2( 1.0f,  1.0f); vec_points[2] = glm::vec4(  0.5f, -0.5f, 0.0f, 1.0f );
    vec_tex_coords[3] = glm::vec2( 1.0f,  1.0f); vec_points[3] = glm::vec4(  0.5f, -0.5f, 0.0f, 1.0f );
    vec_tex_coords[4] = glm::vec2( 0.0f,  0.0f); vec_points[4] = glm::vec4( -0.5f,  0.5f, 0.0f, 1.0f );
    vec_tex_coords[5] = glm::vec2( 1.0f,  0.0f); vec_points[5] = glm::vec4(  0.5f,  0.5f, 0.0f, 1.0f );
  }

  void build_color_map() {
    png_t tex;

    png_init(0, 0);
    png_open_file_read(&tex, "shaders/color_map.png");
    unsigned char * data = new unsigned char[tex.width * tex.height * tex.bpp];
    png_get_data(&tex, data);

    glEnable(GL_TEXTURE_1D);
    glGenTextures(1, &m_colorMapHandle);
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, m_colorMapHandle);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, tex.width, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    png_close_file(&tex);
    delete data;
  }

  void initialize(ias_ss::PheromoneMap * phero_map) {
    glEnable(GL_TEXTURE_1D);
    glEnable(GL_TEXTURE_2D);

    glewInit();

    quad();
    m_phero_map = phero_map;

    build_color_map();
    
    m_program.loadShader("shaders/basic.vert", CGLSLProgram::VERTEX);
    m_program.loadShader("shaders/basic.frag", CGLSLProgram::FRAGMENT);
    m_program.create_link();
    m_program.enable();
    m_program.addUniform("sTexture");
    m_program.addUniform("sColorMap");
    m_program.disable();

    std::cout << "OpenGL Version: " << (char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << (char*)glGetString(GL_VENDOR) << std::endl;
  }

  void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if(m_phero_map != NULL)
      m_textureHandle = m_phero_map->s_build_texture();
    
    m_program.enable();

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glUniform1i(m_program.getLocation("sTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_1D);
    glBindTexture(GL_TEXTURE_1D, m_colorMapHandle);
    glUniform1i(m_program.getLocation("sColorMap"), 1);

    glActiveTexture(GL_TEXTURE0);
    
    glBegin(GL_TRIANGLES); {
      for(int i = 0; i < 6; i++) {
	glTexCoord2f(vec_tex_coords[i].s, vec_tex_coords[i].t);
	glVertex4f(vec_points[i].x, vec_points[i].y, vec_points[i].z, vec_points[i].w);
      }
    } glEnd();

    m_program.disable();
  }

  void display_sensor_map() {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if(m_phero_map != NULL)
      m_sensorTextureHandle = m_phero_map->s_build_sensor_texture();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_sensorTextureHandle);
    
    glBegin(GL_TRIANGLES); {
      for(int i = 0; i < 6; i++) {
	glTexCoord2f(vec_tex_coords[i].s, vec_tex_coords[i].t);
	glVertex4f(vec_points[i].x, vec_points[i].y, vec_points[i].z, vec_points[i].w);
      }
    } glEnd();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  
  void reshape(int w, int h) {
    if(h == 0)
      h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-0.5, 0.5, -0.5, 0.5);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }
}
