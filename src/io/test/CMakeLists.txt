# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(io_tests
  DESCRIPTION "Unit tests for the Manager class")

foreach(TARGET test_io)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_io)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()

