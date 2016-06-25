#pragma once

#ifndef GUI_HPP
#define GUI_HPP

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

#include <string>

class GlGui : public Fl_Gl_Window {
public:
  GlGui(Fl_Window * parent, int x, int y, int w, int h, const char * l);

protected:
  virtual void draw();
  virtual int handle(int);

private:
  Fl_Window * parent;
  std::string title;
  bool initialized;
};

#endif
