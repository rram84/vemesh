# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(sampling_tests
  DESCRIPTION "Unit tests for computing visibility in polygons")

foreach(TARGET test_sampling)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_sampling)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
