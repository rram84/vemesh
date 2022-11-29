# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(slicer_tests
  DESCRIPTION "Unit tests for slicing tet meshes")

file(GLOB DATA_FILES "${CMAKE_SOURCE_DIR}/src/tet-slicer/test/*.tec")
file(COPY ${DATA_FILES} DESTINATION ${CMAKE_BINARY_DIR}/src/tet-slicer/test/)

foreach(TARGET test_zslicer)
  
  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_tet_slicer)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
