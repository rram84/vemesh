# Sriramajayam
#
# Tutorial examples accompanying vemesh.

# Copy sample mesh files (now at the repo root) to the build directory so the
# tutorial executables find them under their working dir (build/tutorial/).
file(COPY
     "${CMAKE_SOURCE_DIR}/sample_data"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# shared tutorial-utilities library 
add_library(vm_tutorial_utils STATIC
  common/vm_tutorial_mesh_slicer.cpp
  common/vm_tutorial_rectangle_mesh.cpp)

target_include_directories(vm_tutorial_utils PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/common")

# the helpers use pmp::SurfaceMesh; pmp's public headers also pull in Eigen,
# whose include path the pmp target does not propagate on its own
target_link_libraries(vm_tutorial_utils PUBLIC pmp Eigen3::Eigen)

target_compile_features(vm_tutorial_utils PUBLIC ${VM_COMPILE_FEATURES})

add_library(vemesh::tutorial_utils ALIAS vm_tutorial_utils)

# tutorial example executables
set(TEST_TARGETS
	element_agglomeration
	vertex_relaxation
	iterative_element_agglomeration
	iterative_vertex_relaxation
	custom_quality_metric
	mesh_slicing
	embed_boundary
	embed_interface)

foreach(TGT IN LISTS TEST_TARGETS)
  add_executable(${TGT} ${TGT}.cpp)

  # vm_tutorial_utils propagates the common/ include directory transitively
  target_link_libraries(${TGT} PRIVATE vm_optimizer vm_tutorial_utils)
  if(OpenMP_FOUND)
	target_link_libraries(${TGT} PRIVATE OpenMP::OpenMP_CXX)
  endif()

  target_compile_features(${TGT} PUBLIC ${VM_COMPILE_FEATURES})

  add_test(NAME ${TGT} COMMAND ${TGT})
endforeach()
