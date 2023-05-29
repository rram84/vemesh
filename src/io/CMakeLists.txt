# Sriramajayam

add_library(vm_io STATIC vm_io.cpp)

# include headers in the current directory
target_include_directories(vm_io PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_io PUBLIC pmp Eigen3::Eigen)

# Add required flags
target_compile_features(vm_io PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_io.h
	vm_io_impl.h
  	DESTINATION ${PROJECT_NAME}/include)
