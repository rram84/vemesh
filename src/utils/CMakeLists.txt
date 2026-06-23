# Sriramajayam

add_library(vm_utils STATIC
		 vm_utils.cpp
		 vm_mesh_inspection.cpp)

add_library(vemesh::vm_utils ALIAS vm_utils)

# include headers in the current directory
target_include_directories(vm_utils PUBLIC
				      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
				       $<INSTALL_INTERFACE:${PROJECT_NAME}/include>)

# link
target_link_libraries(vm_utils PUBLIC pmp Eigen3::Eigen Boost::headers)

if(OpenMP_FOUND)
  target_link_libraries(vm_utils PRIVATE OpenMP::OpenMP_CXX)
endif()

# Add required flags
target_compile_features(vm_utils PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_utils.h
	vm_mesh_inspection.h
  	DESTINATION ${PROJECT_NAME}/include)
