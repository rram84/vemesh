# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(quality_tests
  DESCRIPTION "Unit tests for computing qualities")

foreach(TARGET test_vertex_quality)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_quality)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
