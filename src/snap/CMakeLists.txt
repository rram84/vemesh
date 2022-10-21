# Sriramajayam

add_library(vm_snap STATIC vm_snap.cpp)

# include headers in the current directory
target_include_directories(vm_snap PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_snap PUBLIC pmp Eigen3::Eigen Boost::boost)

# Add required flags
target_compile_features(vm_snap PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_snap.h
  	DESTINATION ${PROJECT_NAME}/include)
