# Sriramajayam

add_library(vm_visibility STATIC vm_visibility.cpp)

# include headers in the current directory
target_include_directories(vm_visibility PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_visibility PUBLIC
				    CGAL::CGAL
				    pmp Eigen3::Eigen
				    Boost::boost
				    vm_vertex_ring
				    vm_io)

# Add required flags
target_compile_features(vm_visibility PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_visibility.h
  	DESTINATION ${PROJECT_NAME}/include)
