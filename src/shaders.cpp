#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#include <cstring>
#include <cstdlib>
#include <iostream>

#include "ecto_gl.hpp"
#include <stdexcept>
namespace ecto_gl
{

  GLuint
  loadShader(GLenum shaderType, const char* pSource)
  {
    GLuint shader = glCreateShader(shaderType);

    if (!shader)
      throw std::logic_error("Could not glCreateShader");

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
      }
      throw std::logic_error("Fail to compile shader.");
    }
    return shader;
  }

  GLuint
  createProgram(GLuint vertexShader, GLuint fragmentShader)
  {
    GLuint program = glCreateProgram();
    if (!program)
      throw std::logic_error("Could not glCreateProgram");
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
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
      throw std::logic_error("Fail to create program.");
    }
    return program;
  }

  GlProgram::GlProgram(const char* vertexSource, const char* fragmentSource)
      :
        vertexShader(loadShader(GL_VERTEX_SHADER, vertexSource)),
        fragmentShader(loadShader(GL_FRAGMENT_SHADER, fragmentSource)),
        program(createProgram(vertexShader, fragmentShader))
  {
  }
  GlProgram::~GlProgram()
  {
    glDeleteProgram(program);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
  }

}
