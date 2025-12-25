# Sriramajayam

add_library(vm_quality STATIC
		 vm_quality_evaluator.cpp
		 vm_face_qualities.cpp
		 vm_vem_stiffness.cpp)

# include headers in the current directory
target_include_directories(vm_quality PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_quality PUBLIC
				 vm_utils)

# Add required flags
target_compile_features(vm_quality PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_quality_evaluator.h
	vm_face_qualities.h
  	DESTINATION ${PROJECT_NAME}/include)
