# Sriramajayam

add_library(vm_tet_slicer STATIC
                 vm_TetMesh.cpp
		 vm_TetMesh_tec.cpp
		 vm_TetMesh_zslice.cpp)

# include headers in the current directory
target_include_directories(vm_tet_slicer PUBLIC
				      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_tet_slicer PUBLIC Boost::boost pmp Eigen3::Eigen)

# Add required flags
target_compile_features(vm_tet_slicer PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_TetMesh.h
  	DESTINATION ${PROJECT_NAME}/include)
