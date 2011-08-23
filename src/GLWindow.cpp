#include <algorithm>
#include <iostream>
#include <string>
#include <GL/glew.h>

#include "ecto_gl.hpp"

//#include <GL/glut.h>
#include <GL/freeglut.h>
#include <GL/glu.h>

#define SHOW_ME() {static unsigned count; std::cout << __PRETTY_FUNCTION__ << ":" << count++ << std::endl;}

namespace ecto_gl
{
  Mouse::Mouse()
      :
        beginx(),
        beginy(),
        state(),
        button(),
        modifiers()
  {
  }
  Mouse::Mouse(int beginx, int beginy, int state, int button, int modifiers)
      :
        beginx(beginx),
        beginy(beginy),
        state(state),
        button(button),
        modifiers(modifiers)
  {
  }
  using Eigen::Vector3f;
  using Eigen::Matrix4f;

  GLWindow::GLWindow(const std::string& windowname)
      :
        windowname_(windowname),
        id_(-1)
  {
  }

  GLWindow::~GLWindow()
  {

  }

  void
  GLWindow::display()
  {
  }

  void
  GLWindow::reshape(int width, int height)
  {
    glViewport(0, 0, width, height);
    camera_.setViewport(width, height);
  }

  void
  GLWindow::mouse(const Mouse& mouse)
  {
    mouse_ = mouse;
  }

  void
  GLWindow::motion(int x, int y)
  {
    Mouse m = mouse_;
    float delta_x = m.beginx - x, delta_y = m.beginy - y;
    if (m.button == GLUT_LEFT_BUTTON && m.state == GLUT_DOWN)
    {
      if (m.modifiers & GLUT_ACTIVE_SHIFT)
      {
        //zoom in and out
        camera_.setFovY(camera_.fovY() * (1.0f + (delta_y / camera_.vpWidth())));
      }
      else
      {
        Eigen::AngleAxisf ry(-delta_x / camera_.vpWidth(), Eigen::Vector3f(0, 1, 0));
        Eigen::AngleAxisf rx(-delta_y / camera_.vpHeight(), Eigen::Vector3f(1, 0, 0));
        Eigen::Quaternionf qx(rx), qy(ry);
        camera_.rotateAroundTarget(qy);
        camera_.rotateAroundTarget(qx);
      }
    }
    else if (m.button == GLUT_MIDDLE_BUTTON && m.state == GLUT_DOWN)
    {

      Vector3f X = camera_.position();
      Eigen::Quaternionf q = camera_.orientation();
      float factor = 10;

      Vector3f dX;
      if (m.modifiers & GLUT_ACTIVE_SHIFT)
      {
        //move along the camera plane normal
        dX = q * Vector3f(factor * delta_x / camera_.vpWidth(), 0, factor * (-delta_y) / camera_.vpHeight());
      }
      else
      {
        //compute the delta x using the the camera orientation,
        //so that the motion is in the plane of the camera.
        dX = q * Vector3f(factor * delta_x / camera_.vpWidth(), factor * (-delta_y) / camera_.vpHeight(), 0);
      }
      camera_.setPosition(X + dX);
    }
    mouse_.beginx = x;
    mouse_.beginy = y;
  }

  void
  GLWindow::timerfunc(int)
  {
  }

  void
  GLWindow::init()
  {
    /* Use depth buffering for hidden surface elimination. */
    glEnable(GL_DEPTH_TEST);
    camera_.setFovY(3.14f / 4);
    camera_.setPosition(Vector3f(0, 0, -5));
    camera_.setTarget(Vector3f(0, 0, 0));
    setView();
    glewInit();
    if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
      std::cout << ("Ready for GLSL\n");

  }

  void
  GLWindow::setView()
  {
    // Readjust view volume
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera_.projectionMatrix().data());
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera_.viewMatrix().data());
  }
  void
  GLWindow::keyboard(unsigned char key, int x, int y)
  {
  }

  void
  GLWindow::destroy()
  {
  }

}
