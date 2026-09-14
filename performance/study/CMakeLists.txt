# performance/study: relaxation-vs-agglomeration study (Computational Mechanics).
#   geometry/ holds the one-time geometry-prep tooling + frozen dataset.
add_subdirectory(geometry)

# vm_study_conditioning: global VEM stiffness assembly + conditioning
# (lambda_max/lambda_2). Assembles from the library element stiffness and solves
# with Spectra (header-only). Factored into a library so it is unit testable.
add_library(vm_study_conditioning STATIC vm_study_conditioning.cpp)
target_link_libraries(vm_study_conditioning PUBLIC vemesh::vm_quality Eigen3::Eigen
                                            PRIVATE vemesh_spectra)
target_include_directories(vm_study_conditioning PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(vm_study_conditioning PUBLIC cxx_std_17)

# mesh_analyze: single-pass per-mesh analysis used by the run harness -- reads a
# mesh ONCE and emits conditioning (G row) + altered-element histograms (H row),
# merging what were two tools (one process spawn + one VTK read per mesh).
add_executable(mesh_analyze mesh_analyze.cpp)
target_link_libraries(mesh_analyze PRIVATE vm_study_conditioning vm_io vemesh::vm_quality vemesh_cli11)
target_compile_features(mesh_analyze PRIVATE cxx_std_17)

# mesh_conditioning: thin CLI over vm_study_conditioning -- a drop-in replacement
# for the MATLAB vem_eig pass, kept for standalone conditioning / validation.
add_executable(mesh_conditioning mesh_conditioning.cpp)
target_link_libraries(mesh_conditioning PRIVATE vm_study_conditioning vm_io vemesh_cli11)
target_compile_features(mesh_conditioning PRIVATE cxx_std_17)

# Unit test for the assembly + conditioning (self-contained; no MATLAB).
add_executable(test_conditioning test/test_conditioning.cpp)
target_link_libraries(test_conditioning PRIVATE vm_study_conditioning)
target_compile_features(test_conditioning PRIVATE cxx_std_17)
add_test(NAME test_conditioning COMMAND test_conditioning)
