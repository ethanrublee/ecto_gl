include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-fpic)
add_library(glew glew.c glewinfo.c visualinfo.c)

