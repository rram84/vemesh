# Sriramajayam

add_library(vm_mesh_slicer STATIC
		 vm_MeshSlicer.cpp)

# include headers in the current directory
target_include_directories(vm_mesh_slicer PUBLIC
				      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_mesh_slicer PUBLIC Boost::boost pmp Eigen3::Eigen)

# Add required flags
target_compile_features(vm_mesh_slicer PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_MeshSlicer.h
  	DESTINATION ${PROJECT_NAME}/include)
