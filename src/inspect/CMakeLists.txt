# Sriramajayam

add_library(vm_inspect STATIC
		 vm_inspect.cpp)

# include headers in the current directory
target_include_directories(vm_inspect PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_inspect PUBLIC vm_vertex_ring pmp Eigen3::Eigen Boost::boost)

# Add required flags
target_compile_features(vm_inspect PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_inspect.h
  	DESTINATION ${PROJECT_NAME}/include)
