#include <iostream>

#include "gui.hpp"

static void redraw_callback(void * arg) {
  Fl_Gl_Window * window = static_cast<Fl_Gl_Window *>(arg);
  window->redraw();
  Fl::repeat_timeout(0.016, redraw_callback, window);
}

GlGui::GlGui(Fl_Window * parent, int x, int y, int w, int h, const char * l) : Fl_Gl_Window(x, y, w, h, l) {
  this->parent = parent;
  title += l;
  initialized = false;
  Fl::add_timeout(0.016, redraw_callback, this);
  resizable(this);
}

void GlGui::draw() {
  if(!valid()) {
    if(!initialized) {
      //opengl::initialize();
      
      std::cout << "OpenGL Version: " << (char*)glGetString(GL_VERSION) << std::endl;
      std::cout << "OpenGL Vendor: " << (char*)glGetString(GL_VENDOR) << std::endl;

      initialized = true;
    }
    //opengl::reshape(w(), h());
  }

  //opengl::display();
}

int GlGui::handle(int event) {
  return Fl_Gl_Window::handle(event);
}
