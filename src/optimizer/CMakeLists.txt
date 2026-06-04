# Sriramajayam

add_library(vm_optimizer STATIC
		 vm_mesh_optimizer.cpp
		 vm_mesh_optimizer_agglomerate.cpp
		 vm_mesh_optimizer_relax.cpp)

add_library(vemesh::vm_optimizer ALIAS vm_optimizer)

# include headers in the current directory
target_include_directories(vm_optimizer PUBLIC
				      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
				      $<INSTALL_INTERFACE:${PROJECT_NAME}/include>) 

# link
target_link_libraries(vm_optimizer PUBLIC
			     vemesh::vm_utils
			     vemesh::vm_io
			     vemesh::vm_quality)

# Optional OpenMP to parallelize quality-evaluation loops 
if(OpenMP_FOUND)
  target_link_libraries(vm_optimizer PRIVATE OpenMP::OpenMP_CXX)
endif()

# Add required flags
target_compile_features(vm_optimizer PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_mesh_optimizer.h
  	DESTINATION ${PROJECT_NAME}/include)
