# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(manager_tests
  DESCRIPTION "Unit tests for the Manager class")

file(GLOB DATA_FILES "${CMAKE_SOURCE_DIR}/src/manager/test/*.dat")
file(COPY ${DATA_FILES} DESTINATION ${CMAKE_BINARY_DIR}/src/manager/test/)

foreach(TARGET test_Manager)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_manager)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
