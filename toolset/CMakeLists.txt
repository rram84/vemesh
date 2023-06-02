# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(toolset
  DESCRIPTION "helpful tools with command line options")

file(GLOB MESH_FILES "${CMAKE_SOURCE_DIR}/src/manager/test/*.OFF")
file(COPY ${MESH_FILES} DESTINATION ${CMAKE_BINARY_DIR}/src/manager/test/)

foreach(TARGET
		convert_mesh_format
		generate_random_square_tri_mesh
		generate_random_circle_tri_mesh
		generate_structured_quad_mesh
		line_clipper
		agglomerator)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_manager CLI11::CLI11 vm_spl_meshes vm_mesh_slicer)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
