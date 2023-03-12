# Sriramajayam

add_library(vm_tri_slicer STATIC
                 vm_TriSlicer.cpp)

# include headers in the current directory
target_include_directories(vm_tri_slicer PUBLIC
				      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_tri_slicer PUBLIC Boost::boost pmp Eigen3::Eigen)

# Add required flags
target_compile_features(vm_tri_slicer PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_TriSlicer.h
  	DESTINATION ${PROJECT_NAME}/include)
