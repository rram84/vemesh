# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(slicer_unit_tests
  DESCRIPTION "Unit tests for the Manager class")

file(GLOB MESH_FILES "${CMAKE_SOURCE_DIR}/test/mesh-slicer/test/*.OFF")
file(COPY ${MESH_FILES} DESTINATION ${CMAKE_BINARY_DIR}/test/mesh-slicer/test/)

foreach(TARGET
	test_tri_slicer
	test_quad_slicer)
	
  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_test_mesh_slicer vm_io vm_test_rect)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
