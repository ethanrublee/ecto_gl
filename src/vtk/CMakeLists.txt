find_package(OpenCV REQUIRED)
FIND_PACKAGE(VTK REQUIRED)
IF(NOT VTK_USE_RENDERING)
  MESSAGE(FATAL_ERROR "Example ${PROJECT_NAME} requires VTK_USE_RENDERING.")
ENDIF(NOT VTK_USE_RENDERING)

INCLUDE(${VTK_USE_FILE})

ADD_EXECUTABLE(Cone Cone.cxx)
TARGET_LINK_LIBRARIES(Cone vtkRendering vtkWidgets)

set( VTK_REQUIRED_LIBS vtkRendering  vtkGraphics vtkHybrid  vtkWidgets 
                     vtkImaging vtkIO vtkFiltering vtkCommon vtkVolumeRendering)

add_executable(plane_render2 plane_render.cpp)
target_link_libraries(plane_render2 ${VTK_REQUIRED_LIBS} ${OpenCV_LIBS} boost_thread)


ectomodule(ecto_sim
     module.cpp
     plane_render.cpp
)

link_ecto(ecto_sim
     ${VTK_REQUIRED_LIBS} ${OpenCV_LIBS}
)
set_target_properties(ecto_sim_ectomodule
    PROPERTIES
    COMPILE_DEFINITIONS "DO_ECTO"
   )
