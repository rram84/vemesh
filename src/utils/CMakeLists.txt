# Sriramajayam

add_library(vm_utils STATIC
		 vm_mesh_inspection.cpp
     	         vm_vertex_ring.cpp)

# include headers in the current directory
target_include_directories(vm_utils PUBLIC
				      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
				       $<INSTALL_INTERFACE:include>)

# link
target_link_libraries(vm_utils PUBLIC pmp Eigen3::Eigen Boost::boost)

# Add required flags
target_compile_features(vm_utils PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_utils.h
	vm_mesh_inspection.h
  	DESTINATION ${PROJECT_NAME}/include)
