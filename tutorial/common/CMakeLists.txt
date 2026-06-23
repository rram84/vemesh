# Sriramajayam
#
# Shared tutorial-utilities library. 

add_library(vm_tutorial_utils STATIC
  vm_tutorial_mesh_slicer.cpp
  vm_tutorial_rectangle_mesh.cpp
  vm_tutorial_polygonSDF.cpp)

target_include_directories(vm_tutorial_utils PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(vm_tutorial_utils PUBLIC pmp Eigen3::Eigen vm_utils)

target_compile_features(vm_tutorial_utils PUBLIC ${VM_COMPILE_FEATURES})

add_library(vemesh::tutorial_utils ALIAS vm_tutorial_utils)
