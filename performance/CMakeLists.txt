# Sriramajayam
#
# In-tree performance tooling, built under BUILD_TESTS alongside the tests and
# tutorials. Not installed.

find_package(CLI11 REQUIRED)

add_executable(vemesh_app
  ${CMAKE_SOURCE_DIR}/app/vemesh_app.cpp
  ${CMAKE_SOURCE_DIR}/app/vemesh_app_cli.cpp)

target_link_libraries(vemesh_app PRIVATE vemesh CLI11::CLI11)

# Optional OpenMP: lets vemesh_app report the thread count in its quality/inspection loops
if(OpenMP_FOUND)
  target_link_libraries(vemesh_app PRIVATE OpenMP::OpenMP_CXX)
endif()

# mesh generators that produce the test datasets
add_subdirectory(mesh_generators)

target_compile_features(vemesh_app PRIVATE cxx_std_17)

# Stage the run script next to the binary on every build (so edits to the source
# script propagate on a plain `make`, not only on re-configure). copy_if_different
# preserves the executable bit and is a no-op when the source is unchanged.
add_custom_target(stage_run_scripts ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_sorgente.sh
          ${CMAKE_CURRENT_BINARY_DIR}/run_sorgente.sh
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/analyze_sorgente.sh
          ${CMAKE_CURRENT_BINARY_DIR}/analyze_sorgente.sh
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_shapes.sh
          ${CMAKE_CURRENT_BINARY_DIR}/run_shapes.sh
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/analyze_shapes.sh
          ${CMAKE_CURRENT_BINARY_DIR}/analyze_shapes.sh
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_circle.sh
          ${CMAKE_CURRENT_BINARY_DIR}/run_circle.sh
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/analyze_circle.sh
          ${CMAKE_CURRENT_BINARY_DIR}/analyze_circle.sh
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          ${CMAKE_CURRENT_SOURCE_DIR}/scripts/plot_sorgente.sh
          ${CMAKE_CURRENT_BINARY_DIR}/plot_sorgente.sh
  COMMENT "Staging performance run scripts")

# Symlink the single build-tree copy of the sample meshes (at ${CMAKE_BINARY_DIR})
# so the run script resolves $HERE/sample_data without duplicating the data.
file(CREATE_LINK ${CMAKE_BINARY_DIR}/sample_data
     ${CMAKE_CURRENT_BINARY_DIR}/sample_data SYMBOLIC)

# Symlink the MATLAB/Octave eigen-analysis sources so the analysis script resolves
# $HERE/matlab without duplicating them (edits to the .m files propagate live).
file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/matlab
     ${CMAKE_CURRENT_BINARY_DIR}/matlab SYMBOLIC)
