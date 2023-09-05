# Sriramajayam

add_library(vm_test_mesh_slicer STATIC
	         vm_test_mesh_slicer.cpp
		 vm_test_mesh_slicer_adjust.cpp
 		 vm_test_mesh_slicer_clip.cpp
		 vm_test_mesh_slicer_embed.cpp)

# include headers in the current directory
target_include_directories(vm_test_mesh_slicer PUBLIC
				      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_test_mesh_slicer PUBLIC Boost::boost pmp Eigen3::Eigen)

# Add required flags
target_compile_features(vm_test_mesh_slicer PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_test_mesh_slicer.h
  	DESTINATION ${PROJECT_NAME}/include)
