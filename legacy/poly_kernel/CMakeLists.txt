# Sriramajayam

add_library(vm_poly_kernel STATIC vm_polygon_kernel.cpp)

# include headers in the current directory
target_include_directories(vm_poly_kernel PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_poly_kernel PUBLIC pmp Eigen3::Eigen Boost::boost)

# Add required flags
target_compile_features(vm_poly_kernel PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_polygon_kernel.h
  	DESTINATION ${PROJECT_NAME}/include)
