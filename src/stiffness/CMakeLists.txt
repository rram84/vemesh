# Sriramajayam

add_library(vm_stiffness STATIC vm_polygon_stiffness.cpp)

# include headers in the current directory
target_include_directories(vm_stiffness PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_stiffness PUBLIC vm_utils)


# Add required flags
target_compile_features(vm_stiffness PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_polygon_stiffness.h
  	DESTINATION ${PROJECT_NAME}/include)
