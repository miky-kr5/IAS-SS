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

#include "gui.hpp"
#include "ogl.hpp"

static const int REFRESH_MS = 0.016; // for 60 fps.

static void redraw_callback(void * arg) {
  GlGui * window = static_cast<GlGui *>(arg);
  window->redraw();
  if(!window->isFinished())
    Fl::repeat_timeout(REFRESH_MS, redraw_callback, window);
}

GlGui::GlGui(Fl_Window * parent, int x, int y, int w, int h, const char * l, PheromoneMap * phero_map) : Fl_Gl_Window(x, y, w, h, l) {
  this->parent = parent;
  this->phero_map = phero_map;
  title += l;
  initialized = false;
  done = false;
}

void GlGui::draw() {
  if(!valid()) {
    if(!initialized) {
      ogl::initialize(phero_map);
      Fl::add_timeout(REFRESH_MS, redraw_callback, this);
      initialized = true;
    }
    ogl::reshape(w(), h());
  }

  ogl::display();
}

int GlGui::handle(int event) {
  return Fl_Gl_Window::handle(event);
}

void GlGui::finish() {
  done = true;
}

bool GlGui::isFinished() {
  return done;
}