# Sriramajayam

add_library(vm_quality STATIC
		 vm_quality_angle.cpp
		 vm_quality_face.cpp
		 vm_quality_vertex.cpp
		 vm_quality_triangles.cpp)

# include headers in the current directory
target_include_directories(vm_quality PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_quality PUBLIC
				 pmp Eigen3::Eigen
				 Boost::boost
				 vm_vertex_ring)

# Add required flags
target_compile_features(vm_quality PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_quality.h
  	DESTINATION ${PROJECT_NAME}/include)
