#include <ecto/ecto.hpp>
#include <GL/glew.h>

#include "ecto_gl.hpp"

#include <vector>
#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/integer.hpp>

#include <GL/gl.h>
#include <GL/glu.h>

#include <GL/freeglut.h>

namespace ecto_gl
{
  using Eigen::Vector3f;
  using Eigen::Matrix4f;

  typedef std::vector<uint8_t> RgbData;
  typedef std::vector<uint16_t> DepthData;

  typedef boost::shared_ptr<RgbData> RgbDataPtr;
  typedef boost::shared_ptr<DepthData> DepthDataPtr;

  typedef boost::shared_ptr<const RgbData> RgbDataConstPtr;
  typedef boost::shared_ptr<const DepthData> DepthDataConstPtr;

  using ecto::tendrils;
  using ecto::spore;

  struct CloudRaw
  {
    static const std::vector<float> uv;

    static const size_t STEP_UV = 2 * sizeof(float); // the step from one point start to the next
    static const size_t PER_UV = 2; //X,Y,Z

    static const size_t STEP_D = sizeof(uint16_t); // the step from one point start to the next
    static const size_t PER_D = 1; //X,Y,Z
    static const size_t STEP_C = 3 * sizeof(char); // the step from one color start to the next
    static const size_t PER_C = 3; //R,G,B

    const uint16_t * depths;
    const float* uvs;
    const uint8_t* colors;
    size_t n; //!< number of points

    CloudRaw()
        :
          depths(0),
          uvs(0),
          colors(0),
          n(0),
          depth_buffer(0),
          color_buffer(0),
          loaded(false)
    {
    }
    CloudRaw(const RgbData& color_buff, const DepthData& depth_buff)
        :
          depths(depth_buff.data()),
          uvs(uv.data()),
          colors(color_buff.data()),
          n(depth_buff.size()),
          depth_buffer(0),
          color_buffer(0),
          loaded(false)
    {

    }

    void
    loadIntoGLBuffer()
    {
      if (loaded)
        return;
      if (n == 0)
        return;
      if (uv_buffer == 0)
      {
        glGenBuffers(1, &uv_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
        glBufferData(GL_ARRAY_BUFFER, STEP_UV * n, uvs, GL_STATIC_DRAW);
      }

      if (depth_buffer == 0)
      {
        glGenBuffers(1, &depth_buffer);
      }
      else
      {
        glDeleteBuffers(1, &depth_buffer);
      }
      glBindBuffer(GL_ARRAY_BUFFER, depth_buffer);
      glBufferData(GL_ARRAY_BUFFER, STEP_D * n, depths, GL_STATIC_DRAW);

      if (color_buffer == 0)
      {
        glGenBuffers(1, &color_buffer);
      }
      else
      {
        glDeleteBuffers(1, &color_buffer);
      }
      glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
      glBufferData(GL_ARRAY_BUFFER, STEP_C * n, colors, GL_STATIC_DRAW);
      loaded = true;
    }

    void
    draw(Camera camera)
    {
      loadIntoGLBuffer();

      glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
      checkGlError("glBindBuffer");
      glEnableVertexAttribArray(uvHandle);
      checkGlError("glEnableVertexAttribArray");
      glVertexAttribPointer(uvHandle, PER_UV, GL_FLOAT, GL_FALSE, STEP_UV, (void*) 0);
      checkGlError("glVertexAttribPointer");

//      glBindBuffer(GL_ARRAY_BUFFER, depth_buffer);
//      checkGlError("glBindBuffer");
//      glEnableVertexAttribArray(depthHandle);
//      checkGlError("glEnableVertexAttribArray");
//      glVertexAttribPointer(depthHandle, PER_D, GL_UNSIGNED_SHORT, GL_FALSE, STEP_D, (void*) 0);
//      checkGlError("glVertexAttribPointer");
//
//      glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
//      checkGlError("glBindBuffer");
//      glEnableVertexAttribArray(colorHandle);
//      checkGlError("glEnableVertexAttribArray");
//      glVertexAttribPointer(colorHandle, PER_C, GL_UNSIGNED_BYTE, GL_FALSE, STEP_C, (void*) 0);
//      checkGlError("glVertexAttribPointer");

//      Matrix4f pt = camera.projectionMatrix() * camera.viewMatrix().matrix();
      //glUniformMatrix4fv(transformHandle, 1, false, pt.data());
      //checkGlError("glUniformMatrix4fv");
      glDrawArrays(GL_POINTS, 0, n);
      checkGlError("glDrawArrays");

    }

    static GLuint uv_buffer;
    GLuint depth_buffer, color_buffer;
    bool loaded;

    static GLuint program, depthHandle, colorHandle, uvHandle;
    static GLint transformHandle;
    static void
    loadProgram()
    {
      static bool visited = false;
      if (visited)
        return;
      visited = true;
      static const char vertexShader[] = SHADER_STR(
          uniform mat4 u_mvpMatrix;
          attribute vec2 a_uv;
          attribute int a_depth;
          attribute vec3 a_color;
          varying vec4 v_color;
          void main()
          {
            float fx = 525.;
            float fy = 525.;
            float cx = 640/2.0 - .5;
            float cy = 480/2.0 - .5;
            vec4 position;
            float z = a_depth / 1000.0;
            position[0] = a_uv[0];
            position[1] = a_uv[1];
            position[2] = z;
            gl_Position = position;
            gl_PointSize = 2.0;
            v_color = vec4(a_color[2]/255.0,a_color[1]/255.0,a_color[0]/255.0,1.0);
          }
      );

      static const char fragmentShader[] = SHADER_STR(
          precision mediump float;
          varying vec4 v_color;
          void main()
          {
            gl_FragColor = v_color;
          };
      );
      program = createProgram(vertexShader, fragmentShader);
      if (!program)
      {
        throw std::logic_error("Could not create program.");
      }

      uvHandle = glGetAttribLocation(program, "a_uv");
      checkGlError("glGetAttribLocation");
      depthHandle = glGetAttribLocation(program, "a_depth");
      checkGlError("glGetAttribLocation");
      colorHandle = glGetAttribLocation(program, "a_color");
      checkGlError("glGetAttribLocation");
      transformHandle = glGetUniformLocation(program, "u_mvpMatrix");
      checkGlError("glGetUniformLocation");
    }
  };
  std::vector<float>
  fill_uv(int w = 640, int h = 480)
  {
    std::vector<float> uv(w * h * 2);
    float* uvp = uv.data();
    for (int v = 0; v < h; v++)
      for (int u = 0; u < w; u++)
      {
        *(uvp++) = u;
        *(uvp++) = v;
      }
    return uv;
  }
  GLuint CloudRaw::uv_buffer = 0;
  const std::vector<float> CloudRaw::uv = fill_uv();
  GLuint CloudRaw::program = 0;
  GLuint CloudRaw::depthHandle = 0;
  GLuint CloudRaw::colorHandle = 0;
  GLuint CloudRaw::uvHandle = 0;
  GLint CloudRaw::transformHandle = 0;

  class CloudWindow: public GLWindow
  {
  public:
    CloudWindow(const std::string window_name)
        :
          GLWindow(window_name)
    {
    }

    RgbDataConstPtr rgb_data;
    DepthDataConstPtr depth_data;
    CloudRaw cloud_raw;
    boost::mutex mtx;

    void
    setData(RgbDataConstPtr c, DepthDataConstPtr d)
    {
      SHOW();
      boost::mutex::scoped_lock lock(mtx);
      rgb_data = c;
      depth_data = d;
      cloud_raw = CloudRaw(*rgb_data, *depth_data);
    }

    virtual void
    display()
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      setView();
      CloudRaw cr;
      {
        SHOW();
        boost::mutex::scoped_lock lock(mtx);
        cr = cloud_raw;
        cr.loadIntoGLBuffer();
      }
      cr.draw(camera_);
      glutSwapBuffers();
    }
    virtual void
    init()
    {
      /* Use depth buffering for hidden surface elimination. */
      glEnable(GL_DEPTH_TEST);
      camera_.setFovY(3.14f / 4);
      Eigen::AngleAxisf aa(3.14f / 4, Eigen::Vector3f(1, 0, 0));
      Eigen::Quaternionf q(aa);
      camera_.setOrientation(q);
      camera_.setPosition(Vector3f(0, 0, -5));
      camera_.setTarget(Vector3f(0, 0, 0));
      setView();
      glewInit();
      if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader)
        std::cout << ("Ready for GLSL") << std::endl;

      CloudRaw::loadProgram();
    }
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
      if (*image_buffer && *depth_buffer)
      {
        if (!window)
        {
          window.reset(new CloudWindow(*window_name));
          ecto_gl::show_window(window);
          std::cout << "Added window" << std::endl;
        }
        window->setData(*image_buffer, *depth_buffer);
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
