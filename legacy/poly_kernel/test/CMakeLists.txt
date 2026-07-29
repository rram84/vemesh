# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(poly_kernel_tests
  DESCRIPTION "Unit tests for computing polygon kernels")

foreach(TARGET test_poly_kernel)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_poly_kernel)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
