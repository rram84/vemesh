# Sriramajayam

add_library(vm_move STATIC
		 vm_vertex_move.cpp)		    

# include headers in the current directory
target_include_directories(vm_move PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_move PUBLIC
			      vm_quality
			      vm_visibility
			      vm_sampling)

# Add required flags
target_compile_features(vm_move PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_vertex_move.h
  	DESTINATION ${PROJECT_NAME}/include)
