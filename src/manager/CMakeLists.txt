# Sriramajayam

add_library(vm_manager STATIC
		 vm_manager.cpp
		 vm_manager_merge.cpp)

# include headers in the current directory
target_include_directories(vm_manager PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_manager PUBLIC vm_merge vm_io vm_quality vm_inspect)

# Add required flags
target_compile_features(vm_manager PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_manager.h
  	DESTINATION ${PROJECT_NAME}/include)
