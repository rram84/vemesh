# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(app DESCRIPTION "Application for running vemesh")

file(GLOB MESH_FILES "${CMAKE_SOURCE_DIR}/src/manager/test/*.OFF")
file(COPY ${MESH_FILES} DESTINATION ${CMAKE_BINARY_DIR}/src/manager/test/)
find_package(OpenMP)
foreach(TARGET
	    vemesh
	    gen_square_mesh
	    clip_mesh
	    annulus_mesh
	    random_mesh
	    multicircle_mesh
	    embed_polygon_mesh
	    threshold
	    moving_mesh)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_manager vm_test_rect
  vm_test_mesh_slicer CLI11::CLI11 OpenMP::OpenMP_CXX)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
