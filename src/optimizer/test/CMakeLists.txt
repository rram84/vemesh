# Sriramajayam

# Unit tests for the Manager class

# Copy sample mesh files to build directory
file(COPY
     "${CMAKE_CURRENT_SOURCE_DIR}/sample_meshes"
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

# List of test executables
set(TEST_TARGETS
    test_mesh_optimizer_helpers
    test_mesh_optimizer)

foreach(TGT IN LISTS TEST_TARGETS)
  add_executable(${TGT} ${TGT}.cpp)

  target_link_libraries(${TGT} PRIVATE vemesh::vm_optimizer)

  target_compile_features(${TGT} PRIVATE ${VM_COMPILE_FEATURES})

  add_test(NAME ${TGT} COMMAND ${TGT})
endforeach()
