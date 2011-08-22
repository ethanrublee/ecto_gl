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

  inline void
  GLWindow::display()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    setView();
    GLfloat n[6][3] =
    { /* Normals for the 6 faces of a cube. */
    { -1.0, 0.0, 0.0 },
    { 0.0, 1.0, 0.0 },
    { 1.0, 0.0, 0.0 },
    { 0.0, -1.0, 0.0 },
    { 0.0, 0.0, 1.0 },
    { 0.0, 0.0, -1.0 } };
    GLint faces[6][4] =
    { /* Vertex indices for the 6 faces of a cube. */
    { 0, 1, 2, 3 },
    { 3, 2, 6, 7 },
    { 7, 6, 5, 4 },
    { 4, 5, 1, 0 },
    { 5, 6, 2, 1 },
    { 7, 4, 0, 3 } };
    GLfloat v[8][3]; /* Will be filled in with X,Y,Z vertexes. */
    /* Setup cube vertex data. */
    v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
    v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
    v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
    v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
    v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
    v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;
    int i;
    for (i = 0; i < 6; i++)
    {
      glBegin(GL_QUADS);
      glNormal3fv(&n[i][0]);
      glVertex3fv(&v[faces[i][0]][0]);
      glVertex3fv(&v[faces[i][1]][0]);
      glVertex3fv(&v[faces[i][2]][0]);
      glVertex3fv(&v[faces[i][3]][0]);
      glEnd();
    }
  }

  inline void
  GLWindow::reshape(int width, int height)
  {
    glViewport(0, 0, width, height);
    camera_.setViewport(width, height);
  }

  inline void
  GLWindow::mouse(const Mouse& mouse)
  {
    mouse_ = mouse;
  }

  inline void
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

  inline void
  GLWindow::timerfunc(int)
  {
    glutPostRedisplay();
  }

  inline void
  GLWindow::init()
  {
    GLfloat light_diffuse[] =
    { 1.0, 0.0, 0.0, 1.0 }; /* Red diffuse light. */
    GLfloat light_position[] =
    { 1.0, 1.0, 1.0, 0.0 }; /* Infinite light location. */
    /* Enable a single OpenGL light. */
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
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

  inline void
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
//    SHOW_ME()
  }

}
