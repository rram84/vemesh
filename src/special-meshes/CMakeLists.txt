# Sriramajayam

add_library(vm_spl_meshes STATIC
		 vm_SpecialMeshes_quad.cpp)

# include headers in the current directory
target_include_directories(vm_spl_meshes PUBLIC
				      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_spl_meshes PUBLIC Boost::boost pmp Eigen3::Eigen)

# Add required flags
target_compile_features(vm_spl_meshes PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_SpecialMeshes.h
  	DESTINATION ${PROJECT_NAME}/include)
