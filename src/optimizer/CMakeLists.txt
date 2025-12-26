# Sriramajayam

add_library(vm_optimizer STATIC
		 vm_mesh_optimizer.cpp
		 vm_mesh_optimizer_merge.cpp
		 vm_mesh_optimizer_move.cpp)
		 
# include headers in the current directory
target_include_directories(vm_optimizer PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_optimizer PUBLIC
			     vm_utils
			     vm_io
			     vm_quality)

# Add required flags
target_compile_features(vm_optimizer PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_mesh_optimizer.h
  	DESTINATION ${PROJECT_NAME}/include)
