# Sriramajayam
#
# Third-party headers provided in-tree.
#
# CLI11 — single-header command-line parser (CLI11.hpp), used by the in-tree
# command-line frontends: vemesh_app and the performance mesh generators.
# Exposed as a header-only INTERFACE target; header-only and build-only, so it
# is not part of the installed vemesh library.
add_library(vemesh_cli11 INTERFACE)
target_include_directories(vemesh_cli11 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Spectra — header-only sparse eigensolver (an Eigen companion). Used only by the
# study tooling (performance/study/mesh_conditioning) to compute VEM stiffness
# conditioning in C++; header-only and build-only, not part of the installed lib.
add_library(vemesh_spectra INTERFACE)
target_include_directories(vemesh_spectra INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/spectra)
