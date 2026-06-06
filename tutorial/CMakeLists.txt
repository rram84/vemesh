# Sriramajayam
#
# Tutorial examples accompanying vemesh.

# Copy sample mesh files (now at the repo root) to the build directory so the
# tutorial executables find them under their working dir (build/tutorial/).
file(COPY
     "${CMAKE_SOURCE_DIR}/sample_data"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# tutorial example executables
set(TEST_TARGETS
	element_agglomeration
	vertex_relaxation
	iterative_element_agglomeration
	iterative_vertex_relaxation
	custom_quality_metric
	mesh_slicing
	embed_boundary
	embed_interface)

foreach(TGT IN LISTS TEST_TARGETS)
  add_executable(${TGT} ${TGT}.cpp)

  target_link_libraries(${TGT} PRIVATE vm_optimizer vm_tutorial_utils)
  if(OpenMP_FOUND)
	target_link_libraries(${TGT} PRIVATE OpenMP::OpenMP_CXX)
  endif()

  target_compile_features(${TGT} PUBLIC ${VM_COMPILE_FEATURES})

  add_test(NAME ${TGT} COMMAND ${TGT})
endforeach()
