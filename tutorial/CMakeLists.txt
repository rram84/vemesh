# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(tutorial DESCRIPTION "Tutorial examples accompanying vemesh")

find_package(OpenMP)

# Copy sample mesh files to build directory
file(GLOB SAMPLE_MESH_FILES "${CMAKE_CURRENT_SOURCE_DIR}/sample_meshes/*")
file(COPY ${SAMPLE_MESH_FILES} DESTINATION "${CMAKE_BINARY_DIR}/sample_meshes")

foreach(TARGET
	    mesh_slicing)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # local sources
 target_sources(${TARGET} PUBLIC
				       "${CMAKE_SOURCE_DIR}/common/vm_tutorial_mesh_slicer.h"
				       "${CMAKE_SOURCE_DIR}/common/vm_tutorial_mesh_slicer.cpp"
				       "${CMAKE_SOURCE_DIR}/common/vm_tutorial_rectangle_mesh.h"
				       "${CMAKE_SOURCE_DIR}/common/vm_tutorial_rectangle_mesh.cpp")

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_optimizer OpenMP::OpenMP_CXX)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
