# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(vm_rect_unit_test
  DESCRIPTION "Unit tests for the rectangle meshes")

foreach(TARGET
	test_rect_mesh)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_test_rect vm_io)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
