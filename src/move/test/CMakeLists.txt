# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(move_tests
  DESCRIPTION "Unit tests for computing visibility in polygons")

foreach(TARGET test_move
	       problem)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_move vm_io)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
