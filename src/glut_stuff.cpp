#include <set>
#include <map>
#include <algorithm>
#include <iostream>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/signals2.hpp>
#include <boost/format.hpp>

#include <GL/glew.h>

#include "ecto_gl.hpp"

//#include <GL/glut.h>
#include <GL/freeglut.h>

#define SHOW_ME() {static unsigned count; std::cout << __PRETTY_FUNCTION__ << ":" << count++ << std::endl;}

namespace ecto_gl
{
  namespace mi = boost::multi_index;

  class GlutContext: boost::noncopyable
  {
    struct GLWindowH
    {
      GLWindowH()
          :
            id(-1)
      {
      }

      explicit
      GLWindowH(int id)
          :
            id(id)
      {
      }
      explicit
      GLWindowH(boost::shared_ptr<GLWindow> w)
          :
            window(w),
            id(w->id_),
            name(w->windowname_)
      {
      }

      bool
      operator<(const GLWindowH& rhs) const
      {
        return id < rhs.id;
      }

      GLWindow::ptr window;
      int id;
      std::string name;

      friend std::ostream&
      operator<<(std::ostream& out, GLWindowH& gh)
      {
        out << "name: " << gh.name << " id: " << gh.id << " @" << gh.window;
        return out;
      }
    };

    typedef boost::multi_index_container<GLWindowH, mi::indexed_by<
    //sort by operator <
        mi::ordered_unique<mi::identity<GLWindowH> >,
        //sort by name
        mi::ordered_non_unique<mi::member<GLWindowH, std::string, &GLWindowH::name> >,
        //sort by unique ptr
        mi::ordered_unique<mi::member<GLWindowH, GLWindow::ptr, &GLWindowH::window> > > //end index
    > WindowSet;
    GlutContext()
        :
          frame_time_(30),
          started_(false),
          quit_(false)
    {
      start();
    }

  public:
    ~GlutContext()
    {
      mlthread_.interrupt();
      mlthread_.join();
    }

    void
    post_window(boost::shared_ptr<GLWindow> gw)
    {
      start();
      boost::mutex::scoped_lock lock(adds_mtx_);
      windowadds_.connect(0, boost::bind(&GlutContext::add_window, this, gw));
    }
    void
    post_remove_window(boost::shared_ptr<GLWindow> gw)
    {
      boost::mutex::scoped_lock lock(adds_mtx_);
      windowadds_.connect(0, boost::bind(&GlutContext::destroyWindow, this, GLWindowH(gw)));
    }

    void
    post_remove_window(const GLWindow& gw)
    {
      boost::mutex::scoped_lock lock(adds_mtx_);
      windowadds_.connect(0, boost::bind(&GlutContext::destroyWindow, this, GLWindowH(gw.id_)));
    }
    void
    wait()
    {
      mlthread_.join();
    }

    void
    stop()
    {
      mlthread_.interrupt();
      wait();
    }

    void
    start()
    {
      if (quit_)
      {
        std::cout << "Quit, restarting." << std::endl;
        stop();
      }
      if (mlthread_ == boost::thread())
        mlthread_ = boost::thread(boost::bind(&GlutContext::mainloop, this));
    }

    static boost::shared_ptr<GLWindow>
    getWindow(int id)
    {
      WindowSet::iterator it = instance().windows_.get<0>().find(GLWindowH(id));
      if (it != instance().windows_.end())
      {
        return it->window;
      }
      else
        return boost::shared_ptr<GLWindow>();
    }
    static GlutContext&
    instance()
    {
      boost::mutex::scoped_lock lock(mtx_);
      if (!instance_)
      {
        instance_.reset(new GlutContext());
      }
      return *instance_;
    }

  private:
    void
    add_window(boost::shared_ptr<GLWindow> gw)
    {
      if (windows_.get<2>().count(gw))
      {
        return;
      }
      int win = glutCreateWindow(&*(gw->windowname_.begin()));
      gw->id_ = win;
      GLWindowH gh(gw);
      windows_.insert(gh);
      glutDisplayFunc(&GlutContext::display);
      glutMouseFunc(&GlutContext::mouse);
      glutReshapeFunc(&GlutContext::reshape);
      glutMotionFunc(&GlutContext::motion);
      glutTimerFunc(frame_time_, &GlutContext::timer, gw->id_);
      glutKeyboardFunc(&GlutContext::keyboard);
      gw->init();
    }

    void
    destroyWindow(GLWindowH win)
    {
      int previous_window = glutGetWindow();
      GLWindow::ptr w = getWindow(win.id);
      glutSetWindow(win.id);
      if (w)
        w->destroy();
      windows_.erase(win);
      glutDestroyWindow(win.id);
      glutSetWindow(previous_window);
    }

    void
    mainloop()
    {
      if (!started_)
      {
        int argc = 1;
        const char * argv[] =
        { "./ecto_glut", 0 };
        glutInit(&argc, const_cast<char**>(argv));
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
        started_ = true;
      }
      quit_ = false;
      while (!boost::this_thread::interruption_requested() && !quit_)
      {
        //http://freeglut.sourceforge.net/docs/api.php
        glutMainLoopEvent();
        {
          boost::mutex::scoped_lock lock(adds_mtx_);
          windowadds_();
          windowadds_.disconnect(0);
        }
        boost::this_thread::sleep(boost::posix_time::microseconds(100));
      }
      destroy_all();
    }

    bool
    started()
    {
      return started_;
    }

    void
    destroy_all()
    {
      while (windows_.begin() != windows_.end())
      {
        destroyWindow(*windows_.begin());
      }
    }

    static void
    display()
    {
      int window = glutGetWindow();
      GLWindow::ptr w = getWindow(window);

      if (w && window != w->id_)
      {
        std::stringstream s;
        s << "current window is not our window!" << window << "!=" << w->id_ << std::endl;
        throw std::logic_error(s.str()); // not sure if this will ever happen.
      }
      if (w)
        w->display();
      glutSwapBuffers();
    }

    static void
    motion(int x, int y)
    {
      int window = glutGetWindow();
      GLWindow::ptr w = getWindow(window);
      if (w)
        w->motion(x, y);
    }

    static void
    reshape(int width, int height)
    {
      int window = glutGetWindow();
      GLWindow::ptr w = getWindow(window);
      if (w)
        w->reshape(width, height);
    }

    static void
    keyboard(unsigned char key, int x, int y)
    {
      int window = glutGetWindow();
      GLWindowH temp(window);
      WindowSet::iterator gh = instance().windows_.find(temp);
      if (gh == instance().windows_.end())
        return;
      if (gh->window)
        gh->window->keyboard(key, x, y);
      return;
    }

    static void
    timer(int val)
    {
      glutSetWindow(val);
      if (val != glutGetWindow())
      {
        instance().windows_.erase(GLWindowH(val));
        return;
      }
      GLWindow::ptr w = getWindow(val);
      if (w)
        w->timerfunc(val);
      glutPostRedisplay();
      glutTimerFunc(instance().frame_time_, &GlutContext::timer, val);
    }

    static void
    mouse(int button, int state, int x, int y)
    {
      int window = glutGetWindow();
      GLWindow::ptr w = getWindow(window);
      Mouse m(x, y, state, button, glutGetModifiers());
      if (w)
        w->mouse(m);
    }

    static boost::shared_ptr<GlutContext> instance_;
    static boost::mutex mtx_;
    boost::mutex adds_mtx_;
    boost::thread mlthread_;
    boost::signals2::signal<void
    (void)> windowadds_;
    WindowSet windows_;
    int frame_time_;
    bool started_, quit_;
  }
  ;

  boost::shared_ptr<GlutContext> GlutContext::instance_;
  boost::mutex GlutContext::mtx_;

  void
  show_window(GLWindow::ptr window)
  {
    GlutContext::instance().post_window(window);
  }

  void
  destroy_window(GLWindow::ptr window)
  {
    GlutContext::instance().post_remove_window(window);
  }

  void
  destroy_window(const GLWindow& window)
  {
    GlutContext::instance().post_remove_window(window);
  }

  void
  wait()
  {
    GlutContext::instance().wait();
  }

  void
  stop()
  {
    GlutContext::instance().stop();

  }

  int
  checkGlError(std::ostream& out)
  {
    GLint error_any = 0;
    for (GLint error = glGetError(); error; error = glGetError())
    {
      error_any = error;
      out << gluErrorString(error) << std::endl;
    }
    return error_any;
  }

}
#if _IS_MAIN_
int
main()
{
  using namespace ecto_gl;
  {
    boost::shared_ptr<GLWindow> gw(new GLWindow("a box"));
    boost::shared_ptr<GLWindow> gwb(new GLWindow("another box"));

    ecto_gl::show_window(gw);
    ecto_gl::show_window(gwb);
    ecto_gl::wait();
  }
  std::cout << "second round" << std::endl;
  {
    boost::shared_ptr<GLWindow> gw(new GLWindow("a destitute box"));
    ecto_gl::show_window(gw);
    ecto_gl::destroy_window(gw);
    std::cout << "destroy.." << std::endl;
    ecto_gl::wait();
    ecto_gl::show_window(gw);
    ecto_gl::wait();
  }

  std::cout << "exiting " << std::endl;
  return 0;
}
#endif
