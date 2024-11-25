# Sriramajayam

add_library(vm_manager STATIC
		 vm_Manager.cpp
		 vm_Manager_merge.cpp
		 vm_Manager_move.cpp)
		 
# include headers in the current directory
target_include_directories(vm_manager PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_manager PUBLIC
			     vm_utils
			     vm_io
			     vm_quality)

# Add required flags
target_compile_features(vm_manager PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_Manager.h
  	DESTINATION ${PROJECT_NAME}/include)
