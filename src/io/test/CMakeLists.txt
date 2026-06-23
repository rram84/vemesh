# Sriramajayam

cmake_minimum_required(VERSION 3.17)

project(io_tests
  DESCRIPTION "Unit tests for mesh I/O")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
# List of test executables
set(TEST_TARGETS test_io)

foreach(TGT IN LISTS TEST_TARGETS)
  add_executable(${TGT} ${TGT}.cpp)

  target_link_libraries(${TGT} PRIVATE vemesh::vm_io)

  target_compile_features(${TGT} PRIVATE ${VM_COMPILE_FEATURES})

  add_test(NAME ${TGT} COMMAND ${TGT})
endforeach()

