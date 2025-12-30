# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(optimizer_tests
  DESCRIPTION "Unit tests for the Manager class")

# Copy sample mesh files to build directory
file(COPY
     "${CMAKE_CURRENT_SOURCE_DIR}/sample_meshes"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

 
foreach(TARGET
	test_mesh_optimizer_helpers)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_optimizer)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
