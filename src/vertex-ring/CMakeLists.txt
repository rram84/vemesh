# Sriramajayam

add_library(vm_vertex_ring STATIC vm_vertex_ring.cpp)

# include headers in the current directory
target_include_directories(vm_vertex_ring PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_vertex_ring PUBLIC pmp Eigen3::Eigen Boost::boost)

# Add required flags
target_compile_features(vm_vertex_ring PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_vertex_ring.h
  	DESTINATION ${PROJECT_NAME}/include)
