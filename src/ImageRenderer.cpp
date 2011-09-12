#include <ecto/ecto.hpp>
#include <GL/glew.h>

#include "ecto_gl.hpp"

#include <vector>
#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/integer.hpp>
#include <sstream>

#include <GL/gl.h>
#include <GL/glu.h>

#include <GL/freeglut.h>

namespace ecto_gl
{
  using Eigen::Vector3f;
  using Eigen::Matrix4f;
  using ecto::tendrils;
  using ecto::spore;
  struct ImageProgram
  {
    ImageProgram()
    {
      static const char vertexShader[] = SHADER_STR(
          attribute float depth;
          attribute vec3 rgb;
          varying vec4 color;
          uniform mat4 projection_modelview;
          void main()
          {
            float fx = 525.;
            float fy = 525.;
            float cx = 640./2.0 - .5;
            float cy = 480./2.0 - .5;

            float y = gl_VertexID/640;
            float x = gl_VertexID%640;

            vec4 position;
            float d = depth / 1000.;
            position[0] = (x - cx)*d/fx;
            position[1] = (y - cy)*d/fy;
            position[2] = d;
            position[3] = 1;

            color = vec4(rgb[0]/255.,rgb[1]/255.,rgb[2]/255.,1.);
            gl_Position = projection_modelview*position;
            gl_PointSize = 2.0;
          }
      );

      static const char fragmentShader[] = SHADER_STR(
          precision mediump float;
          varying vec4 color;
          void main()
          {
            gl_FragColor = color;
          };
      );
      program.reset(new GlProgram(vertexShader, fragmentShader));
      projection_modelview = glGetUniformLocation(program->program, "projection_modelview");
      depthHandle = glGetAttribLocation(program->program, "depth");
      rgbHandle = glGetAttribLocation(program->program, "rgb");

      CHECK_GLUT_ERROR
    }
    boost::shared_ptr<GlProgram> program;
    GLuint projection_modelview, depthHandle, rgbHandle;
  };

//  std::vector<float>
//  fill_uv(int w = 640, int h = 480)
//  {
//    std::vector<float> uv(w * h * 2);
//    float* uvp = uv.data();
//    for (int v = 0; v < h; v++)
//      for (int u = 0; u < w; u++)
//      {
//        *(uvp++) = u;
//        *(uvp++) = v;
//      }
//    return uv;
//  }

  struct CloudData: boost::noncopyable
  {
//    static const size_t PER_UV = 2; //U,V
//    static const size_t STEP_UV = PER_UV * sizeof(float); // the step from one point start to the next

    static const size_t PER_DEPTH = 1; //depth
    static const size_t STEP_DEPTH = PER_DEPTH * sizeof(uint16_t); // the step from one point start to the next

    static const size_t PER_RGB = 3; //depth
    static const size_t STEP_RGB = PER_RGB * sizeof(uint8_t); // the step from one point start to the next

    CloudData()
        :
          n(640 * 480),
          depth_buffer(0),
          rgb_buffer(0)
    {

    }
    ~CloudData()
    {
      glDeleteBuffers(1, &depth_buffer);
      glDeleteBuffers(1, &rgb_buffer);

      CHECK_GLUT_ERROR
    }
    void
    setDepth(const DepthData& depth)
    {
      if (!glIsBuffer(depth_buffer))
      {
        glGenBuffers(1, &depth_buffer);
      }
      glBindBuffer(GL_ARRAY_BUFFER, depth_buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(uint16_t) * depth.size(), depth.data(), GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      CHECK_GLUT_ERROR


    }

    void
    setColor(const RgbData& rgb)
    {
      if (!glIsBuffer(rgb_buffer))
      {
        glGenBuffers(1, &rgb_buffer);
      }
      glBindBuffer(GL_ARRAY_BUFFER, rgb_buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(uint8_t) * rgb.size(), rgb.data(), GL_DYNAMIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);

      CHECK_GLUT_ERROR
    }
    void
    draw(const Camera& c)
    {
      glViewport(0, 0, c.vpWidth(), c.vpHeight());
      if(!glIsProgram(program.program->program))
      {
        std::cout << "not a program" << std::endl;
        program = CloudProgram();
      }
      glUseProgram(program.program->program);
      if (glIsBuffer(depth_buffer))
      {
        glEnableVertexAttribArray(program.depthHandle);
        glBindBuffer(GL_ARRAY_BUFFER, depth_buffer);
        glVertexAttribPointer(program.depthHandle, PER_DEPTH, GL_UNSIGNED_SHORT, GL_FALSE, STEP_DEPTH, (void*) 0);
      }
      CHECK_GLUT_ERROR

      if (glIsBuffer(rgb_buffer))
      {
        glEnableVertexAttribArray(program.rgbHandle);
        glBindBuffer(GL_ARRAY_BUFFER, rgb_buffer);
        glVertexAttribPointer(program.rgbHandle, PER_RGB, GL_UNSIGNED_BYTE, GL_FALSE, STEP_RGB, (void*) 0);
      }
      CHECK_GLUT_ERROR

      Matrix4f p = c.projectionMatrix() * c.viewMatrix().matrix();
      glUniformMatrix4fv(program.projection_modelview, 1, false, p.data());

      CHECK_GLUT_ERROR

      glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
      glDrawArrays(GL_POINTS, 0, 2 * n);
      CHECK_GLUT_ERROR
      glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glUseProgram(0);

      CHECK_GLUT_ERROR
    }
    std::vector<float> uvs;
    int n;
    GLuint uv_buffer, depth_buffer, rgb_buffer;
    CloudProgram program;
  };

  class CloudWindow: public GLWindow
  {
  public:
    CloudWindow(const std::string window_name)
        :
          GLWindow(window_name),
          quit(false)
    {
    }

    virtual void
    display()
    {
      glEnable(GL_DEPTH_TEST);
      glDepthRange(0.1, 100);
      glClearColor(0.0f, 0.0f, 0.0f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if (!cloud_raw.get())
        cloud_raw.reset(new CloudData);

      if (depth)
      {
        boost::mutex::scoped_lock lock(mtx);
        cloud_raw->setDepth(*depth);
        depth.reset();
        cloud_raw->setColor(*rgb);
        rgb.reset();
      }
      cloud_raw->draw(camera_);

      CHECK_GLUT_ERROR
    }
    virtual void
    init()
    {
      cloud_raw.reset();
      /* Use depth buffering for hidden surface elimination. */
      camera_.setFovY(3.14f / 4);
      camera_.setPosition(Vector3f(0, 0, -1));
      camera_.setTarget(Vector3f(0, 0, 0));
      Eigen::AngleAxisf aa(M_PI, Eigen::Vector3f(0, 0, 1));
      Eigen::Quaternionf q(aa);
      aa = Eigen::AngleAxisf(M_PI, Eigen::Vector3f(0, 1, 0));
      q *= Eigen::Quaternionf(aa);
      camera_.setOrientation(q);
      glewInit();

      CHECK_GLUT_ERROR
    }

    void
    timerfunc(int)
    {
    }

    void
    keyboard(unsigned char key, int x, int y)
    {
      switch (key)
      {
        case 'q':
          quit = true;
          break;
        default:
          break;
      }
      return;
    }

    void
    destroy()
    {
//      cloud_raw.reset();
    }

    std::auto_ptr<CloudData> cloud_raw;
    DepthDataConstPtr depth;
    RgbDataConstPtr rgb;
    boost::mutex mtx;
    bool quit;
  };
  struct PointCloudDisplay
  {
    static void
    declare_params(tendrils& params)
    {
      params.declare<std::string>("window_name", "A name for the window.", "cloudy.");
    }

    static void
    declare_io(const tendrils& params, tendrils& i, tendrils& o)
    {
      i.declare<int>("depth_width", "Depth frame width.");
      i.declare<int>("depth_height", "Depth frame height.");
      i.declare<int>("image_width", "Image frame width.");
      i.declare<int>("image_height", "Image frame height.");
      i.declare<int>("image_channels", "Number of image channels.");
      i.declare<DepthDataConstPtr>("depth_buffer");
      i.declare<RgbDataConstPtr>("image_buffer");
    }

    void
    configure(const tendrils& p, const tendrils& i, const tendrils& o)
    {
      depth_height = i["depth_height"];
      depth_width = i["depth_width"];
      image_width = i["image_width"];
      image_height = i["image_height"];
      image_channels = i["image_channels"];
      image_buffer = i["image_buffer"];
      depth_buffer = i["depth_buffer"];
      window_name = p["window_name"];
    }

    int
    process(const tendrils&, const tendrils&)
    {
      DepthDataConstPtr db = *depth_buffer;
      RgbDataConstPtr cb = *image_buffer;

      if (!window)
      {
        window.reset(new CloudWindow(*window_name));
      }

      if (window->quit)
      {
        ecto_gl::stop();
        return ecto::QUIT;
      }

      ecto_gl::show_window(window);
      if (cb && db)
      {
        window->setData(cb, db);
      }
      return ecto::OK;
    }

    ecto::spore<int> depth_width, depth_height, image_width, image_height, image_channels;
    ecto::spore<DepthDataConstPtr> depth_buffer;
    ecto::spore<RgbDataConstPtr> image_buffer;
    ecto::spore<std::string> window_name;

    boost::shared_ptr<CloudWindow> window;
  };
}
ECTO_CELL(ecto_gl, ecto_gl::PointCloudDisplay, "PointCloudDisplay", "A glut point cloud viewer")
