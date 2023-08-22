# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(vem-ppt
  DESCRIPTION "example for ppt")

foreach(TARGET
		create_vertices
		create_poly
		agglomerator-vis)
		

  # add executable
  add_executable(${TARGET} ${TARGET}.cpp)

  # link 
  target_link_libraries(${TARGET} PUBLIC vm_manager Boost::boost vm_spl_meshes vm_mesh_slicer)

  # choose appropriate flags
  target_compile_features(${TARGET} PUBLIC ${COMPILE_FEATURES})

endforeach()
