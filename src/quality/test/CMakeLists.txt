# Sriramajayam

# Unit tests for quality calculations

# List of test executables
set(TEST_TARGETS
	test_vem_quality
	test_geom_qualities
	test_quality_evaluator)
	
foreach(TGT IN LISTS TEST_TARGETS)
  add_executable(${TGT} ${TGT}.cpp)

  target_link_libraries(${TGT} PRIVATE vemesh::vm_quality)

  target_compile_features(${TGT} PRIVATE ${VM_COMPILE_FEATURES})

  add_test(NAME ${TGT} COMMAND ${TGT})
endforeach()
