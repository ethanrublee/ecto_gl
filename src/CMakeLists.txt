find_package(GLUT)
find_package(OpenGL)
find_package(Eigen)
find_package(OpenCV REQUIRED)

option(GL_CMAKE_VERBOSE TRUE)

if(${GL_CMAKE_VERBOSE})
    message(STATUS "****** GLUT includes: ${GLUT_INCLUDE_DIR}")
    message(STATUS "****** OpenGL includes: ${OPENGL_INCLUDE_DIR}")
    message(STATUS "****** Eigen includes: ${EIGEN_INCLUDE_DIRS}")
    message(STATUS "****** Boost includes: ${Boost_INCLUDE_DIR}")
    message(STATUS "^^^^^ GLUT_FOUND ${GLUT_FOUND}")
    message(STATUS "^^^^^ OPENGL_FOUND ${OPENGL_FOUND}")
    message(STATUS "^^^^^ EIGEN_FOUND ${EIGEN_FOUND}")
    
    message(STATUS "****** GLUT_LIBRARY: ${GLUT_LIBRARY}")
    message(STATUS "****** OPENGL_LIBRARY: ${OPENGL_LIBRARY}")
endif()

if(NOT GLUT_FOUND OR NOT OPENGL_FOUND OR NOT EIGEN_FOUND)
    message(STATUS "^^^^^ Disabling ecto_gl as can't find dependencies")
    return()
endif()

find_package(Boost COMPONENTS
  thread
  REQUIRED
  )

include_directories(
    ${GLUT_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${EIGEN_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/glew
    )

add_subdirectory(glew)

ectomodule(ecto_gl
     module.cpp
     PointCloudRender.cpp
     glut_stuff.cpp
     camera.cpp
     GLWindow.cpp
     shaders.cpp
)

link_ecto(ecto_gl
    glew
    ${GLUT_LIBRARY}
    ${OPENGL_LIBRARY}
    ${Boost_LIBRARIES}
)

add_subdirectory(vtk)
