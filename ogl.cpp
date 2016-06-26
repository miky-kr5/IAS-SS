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

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace ogl
{ 
  // Variables
  static CGLSLProgram m_program;
  static PheromoneMap * m_phero_map = NULL;
  static GLuint m_textureHandle;

  // Quad definition
  static glm::vec4 vec_points[6];
  static glm::vec2 vec_tex_coords[6];
  
  static void quad() {
    vec_tex_coords[0] = glm::vec2( 0.0f,  0.0f); vec_points[0] = glm::vec4( -0.5f, -0.5f, 0.0f, 1.0f );
    vec_tex_coords[1] = glm::vec2( 0.0f,  1.0f); vec_points[1] = glm::vec4( -0.5f,  0.5f, 0.0f, 1.0f );
    vec_tex_coords[2] = glm::vec2( 1.0f,  0.0f); vec_points[2] = glm::vec4(  0.5f, -0.5f, 0.0f, 1.0f );
    vec_tex_coords[3] = glm::vec2( 1.0f,  0.0f); vec_points[3] = glm::vec4(  0.5f, -0.5f, 0.0f, 1.0f );
    vec_tex_coords[4] = glm::vec2( 0.0f,  1.0f); vec_points[4] = glm::vec4( -0.5f,  0.5f, 0.0f, 1.0f );
    vec_tex_coords[5] = glm::vec2( 1.0f,  1.0f); vec_points[5] = glm::vec4(  0.5f,  0.5f, 0.0f, 1.0f );
  }

  void initialize(PheromoneMap * phero_map) {
    glewInit();

    quad();
    m_phero_map = phero_map;
    
    m_program.loadShader("shaders/basic.vert", CGLSLProgram::VERTEX);
    m_program.loadShader("shaders/basic.frag", CGLSLProgram::FRAGMENT);
    m_program.create_link();
    m_program.enable();
    m_program.addUniform("sTexture");
    m_program.disable();

    std::cout << "OpenGL Version: " << (char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << (char*)glGetString(GL_VENDOR) << std::endl;
  }

  void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    if(m_phero_map != NULL)
      m_textureHandle = m_phero_map->s_build_texture(); // Good grief!
    
    m_program.enable();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_textureHandle);
    glUniform1i(m_program.getLocation("sTexture"), 0);

    glBegin(GL_TRIANGLES); {
      for(int i = 0; i < 6; i++) {
	glTexCoord2f(vec_tex_coords[i].s, vec_tex_coords[i].t);
	glVertex4f(vec_points[i].x, vec_points[i].y, vec_points[i].z, vec_points[i].w);
      }
    } glEnd();

    m_program.disable();
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
