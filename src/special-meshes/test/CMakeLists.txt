# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(spl_meshes_tests
  DESCRIPTION "Unit tests for the special meshes")

foreach(TARGET
	test_rect_mesh
	test_delaunay)

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_spl_meshes vm_io CGAL::CGAL)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
