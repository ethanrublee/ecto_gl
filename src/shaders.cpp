#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <cstring>
#include <cstdlib>
#include <iostream>

#include "ecto_gl.hpp"
namespace ecto_gl
{

  GLuint
  loadShader(GLenum shaderType, const char* pSource)
  {
    GLuint shader = glCreateShader(shaderType);
    if (shader)
    {
      glShaderSource(shader, 1, &pSource, NULL);
      glCompileShader(shader);
      GLint compiled = 0;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
      if (!compiled)
      {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen)
        {
          char* buf = (char*) malloc(infoLen);
          if (buf)
          {
            glGetShaderInfoLog(shader, infoLen, NULL, buf);
            std::cerr << "Could not compile shader:\n" << shaderType << " " << buf << std::endl;
            free(buf);
          }
          glDeleteShader(shader);
          shader = 0;
        }
      }
    }
    return shader;
  }

  GLuint
  createProgram(const char* pVertexSource, const char* pFragmentSource)
  {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader)
    {
      return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader)
    {
      return 0;
    }

    GLuint program = glCreateProgram();
    if (program)
    {
      glAttachShader(program, vertexShader);
      glAttachShader(program, pixelShader);
      glLinkProgram(program);
      GLint linkStatus = GL_FALSE;
      glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
      if (linkStatus != GL_TRUE)
      {
        GLint bufLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
        if (bufLength)
        {
          char* buf = (char*) malloc(bufLength);
          if (buf)
          {
            glGetProgramInfoLog(program, bufLength, NULL, buf);
            std::cerr << "Could not link program:\n" << buf << std::endl;
            free(buf);
          }
        }
        glDeleteProgram(program);
        program = 0;
      }
    }
    return program;
  }
}
