# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(utils_tests
  DESCRIPTION "Unit tests for utilities")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/ring.off" DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

foreach(TARGET test_vertex_ring)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_utils vm_io)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
