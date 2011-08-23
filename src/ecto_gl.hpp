#pragma once
#include <boost/shared_ptr.hpp>
#include <string>
#include "camera.h"
#include <GL/gl.h>

namespace ecto_gl
{
  struct Mouse
  {
    Mouse();
    Mouse(int beginx, int beginy, int state, int button, int modifiers);
    int beginx, beginy;
    int state;
    int button;
    int modifiers;
  };

  class GLWindow
  {
  public:
    GLWindow(const std::string& windowname);

    virtual
    ~GLWindow();

    std::string windowname_;
    int id_;

    Camera camera_;
    Mouse mouse_;

    virtual void
    display();

    virtual void
    reshape(int width, int height);

    virtual void
    mouse(const Mouse& mouse);

    virtual void
    motion(int x, int y);

    virtual void
    timerfunc(int val);

    virtual void
    init();

    virtual void
    setView();

    virtual void
    keyboard(unsigned char key, int x, int y);

    virtual void
    destroy();

    typedef boost::shared_ptr<GLWindow> ptr;
    typedef boost::shared_ptr<const GLWindow> const_ptr;
  };

  void
  show_window(GLWindow::ptr window);
  void
  destroy_window(const GLWindow& window);

  void
  destroy_window(GLWindow::ptr window);

  void
  wait();
  void
  stop();

  struct GlProgram
  {
    GlProgram(const char* pVertexSource, const char* pFragmentSource);
    ~GlProgram();
    GLuint vertexShader, fragmentShader;
    GLuint program;
  };

  int
  checkGlError(std::ostream& out);

}

#define SHADER_STR(A) #A

#define CHECK_GLUT_ERROR { if(checkGlError(std::cerr)) std::cerr << "where: " <<__FILE__<<":"<<__LINE__ << std::endl; }

