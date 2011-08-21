find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Eigen REQUIRED)

find_package(Boost COMPONENTS
  thread
  REQUIRED
  )
  

add_definitions(-Wall -Werror)

if(GCC_VERSION VERSION_GREATER 4.4 OR GCC_VERSION VERSION_EQUAL 4.4)
  list(APPEND FASTIDIOUS_FLAGS -Wl,--Wno-undefined)
endif()
add_subdirectory(glew)

include_directories(${GLUT_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIRS}
    ${EIGEN_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/glew
    )

add_executable(gldemo glut_stuff.cpp
     camera.cpp
     GLWindow.cpp
     shaders.cpp)
     
set_target_properties(gldemo
  PROPERTIES
  COMPILE_FLAGS -D_IS_MAIN_=1
  )

target_link_libraries(gldemo ${GLUT_LIBRARY} ${OPENGL_LIBRARY} boost_thread glew)

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
    ${GLUT_LIBRARY} ${OPENGL_LIBRARY} ${Boost_LIBRARIES} 
)
