cmake_minimum_required(VERSION 2.8)
project(ecto_gl)

find_package(ecto REQUIRED)

find_package(ecto_ros REQUIRED)

message(STATUS ${ecto_ros_INCLUDE_DIRS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
ecto_python_env_gen(${CMAKE_BINARY_DIR}/lib)

add_subdirectory(src)