# Sriramajayam

cmake_minimum_required(VERSION 3.17)

project(tutorial DESCRIPTION "Tutorial examples accompanying vemesh")

find_package(OpenMP)

# Copy sample mesh files to build directory
file(COPY
     "${CMAKE_CURRENT_SOURCE_DIR}/sample_data"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/common")

# List of test executables
set(TEST_TARGETS
	agglomerate
	mesh_slicing
	vemesh_app)
	
foreach(TGT IN LISTS TEST_TARGETS)
  add_executable(${TGT} ${TGT}.cpp)

  # local sources
 target_sources(${TGT} PRIVATE
				       "${CMAKE_CURRENT_SOURCE_DIR}/common/vm_tutorial_mesh_slicer.h"
				       "${CMAKE_CURRENT_SOURCE_DIR}/common/vm_tutorial_mesh_slicer.cpp"
				       "${CMAKE_CURRENT_SOURCE_DIR}/common/vm_tutorial_rectangle_mesh.h"
				       "${CMAKE_CURRENT_SOURCE_DIR}/common/vm_tutorial_rectangle_mesh.cpp")


  target_include_directories(${TGT} PUBLIC ${COMMON_DIR})
  target_link_libraries(${TGT} PRIVATE vm_optimizer)
  if(OpenMP_FOUND)
	target_link_libraries(${TGT} PRIVATE OpenMP::OpenMP_CXX)
  endif()
  
  target_compile_features(${TGT} PUBLIC ${VM_COMPILE_FEATURES})

  add_test(NAME ${TGT} COMMAND ${TGT})
endforeach()
