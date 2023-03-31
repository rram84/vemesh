# Sriramajayam

add_library(vm_merge STATIC vm_face_merge.cpp)

# include headers in the current directory
target_include_directories(vm_merge PUBLIC
					$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# link
target_link_libraries(vm_merge PUBLIC vm_inspect vm_quality)

# Add required flags
target_compile_features(vm_merge PUBLIC ${VM_COMPILE_FEATURES})

# install headers
install(FILES
	vm_face_merge.h
  	DESTINATION ${PROJECT_NAME}/include)
