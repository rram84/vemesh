# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(stiffness_tests
  DESCRIPTION "Unit tests for computing stiffness for polygons")

foreach(TARGET
		test_stiffness)
	        test_sutton)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_stiffness)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
