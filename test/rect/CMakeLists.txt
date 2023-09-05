# Sriramajayam

add_library(vm_test_rect STATIC
		 vm_test_rectangle_mesh.cpp)

# include headers in the current directory
target_include_directories(vm_test_rect PUBLIC
				      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_test_rect PUBLIC pmp Eigen3::Eigen)

# Add required flags
target_compile_features(vm_test_rect PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_test_rectangle_mesh.h
  	DESTINATION ${PROJECT_NAME}/include)
