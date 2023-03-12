# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(tri_slice_tests
  DESCRIPTION "Unit tests for the Manager class")

file(GLOB MESH_FILES "${CMAKE_SOURCE_DIR}/src/tri-slicer/test/*.OFF")
file(COPY ${MESH_FILES} DESTINATION ${CMAKE_BINARY_DIR}/src/tri-slicer/test/)

foreach(TARGET
	test_tri_slicer)
	
  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_tri_slicer vm_io)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
