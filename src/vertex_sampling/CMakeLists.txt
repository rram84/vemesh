# Sriramajayam

add_library(vm_vert_sampling STATIC vm_vert_sampling.cpp)

# include headers in the current directory
target_include_directories(vm_vert_sampling PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_vert_sampling PUBLIC Boost::boost)

# Add required flags
target_compile_features(vm_vert_sampling PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_vert_sampling.h
  	DESTINATION ${PROJECT_NAME}/include)
