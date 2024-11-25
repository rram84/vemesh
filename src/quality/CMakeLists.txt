# Sriramajayam

add_library(vm_quality STATIC
		 vm_face_quality_angle.cpp
		 vm_face_quality_stiffness.cpp
		 vm_vertex_quality_stiffness.cpp
		 vm_vertex_quality_shape.cpp)

# include headers in the current directory
target_include_directories(vm_quality PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_quality PUBLIC
				 pmp Eigen3::Eigen
				 Boost::boost
				 vm_utils
				 vm_stiffness)

# Add required flags
target_compile_features(vm_quality PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_face_quality.h
	vm_vertex_quality.h
  	DESTINATION ${PROJECT_NAME}/include)
