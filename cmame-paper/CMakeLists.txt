# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(manager_tests
  DESCRIPTION "Unit tests for the Manager class")

file(GLOB MESH_FILES "${CMAKE_SOURCE_DIR}/src/manager/test/*.OFF")
file(COPY ${MESH_FILES} DESTINATION ${CMAKE_BINARY_DIR}/src/manager/test/)

foreach(TARGET
		rect_mesh_problem1)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_manager CLI11::CLI11 vm_spl_meshes vm_mesh_slicer)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
