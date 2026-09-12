# Sriramajayam
#
# Study tooling for the Computational Mechanics paper: relaxation vs.
# agglomeration workflows over embedded closed geometries. Built under
# BUILD_TESTS alongside the other performance tooling; not installed.

# Geometry oversampler: .fset piecewise-Bezier feature sets -> polygonal .dat.
# Needs only CLI11 (header-only via vemesh_cli11); no library dependencies.
add_executable(fset_to_polygon fset_to_polygon.cpp)
target_link_libraries(fset_to_polygon PRIVATE vemesh_cli11)
target_compile_features(fset_to_polygon PRIVATE cxx_std_17)

# Background generator: fixed structured triangle/quad meshes over [lo,hi]^2.
# vm_utils arrives transitively via vm_tutorial_utils (PUBLIC).
add_executable(make_background make_background.cpp)
target_link_libraries(make_background PRIVATE vm_io vm_tutorial_utils vemesh_cli11)
target_compile_features(make_background PRIVATE cxx_std_17)

# Admissibility gate: does a geometry embed robustly under node perturbations?
add_executable(embed_check embed_check.cpp)
target_link_libraries(embed_check PRIVATE vm_io vm_tutorial_utils vemesh_cli11)
target_compile_features(embed_check PRIVATE cxx_std_17)

# A-priori resolution gate: does the nodal-sampled SDF faithfully represent the
# geometry, or does a cell carry >1 interface branch? (read_polygon_loops)
add_executable(sampling_check sampling_check.cpp)
target_link_libraries(sampling_check PRIVATE vm_tutorial_utils vemesh_cli11)
target_compile_features(sampling_check PRIVATE cxx_std_17)

# Salvage failing geometries: remove smallest offending loops until admissible.
add_executable(trim_geometry trim_geometry.cpp)
target_link_libraries(trim_geometry PRIVATE vm_tutorial_utils vemesh_cli11)
target_compile_features(trim_geometry PRIVATE cxx_std_17)

# Mesh-independent coarsening: morphological close-then-open at scale delta.
add_executable(coarsen_geometry coarsen_geometry.cpp)
target_link_libraries(coarsen_geometry PRIVATE vm_tutorial_utils vemesh_cli11)
target_compile_features(coarsen_geometry PRIVATE cxx_std_17)
