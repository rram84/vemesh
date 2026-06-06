# Sriramajayam
#
# In-tree performance tooling, built under BUILD_TESTS alongside the tests and
# tutorials. Not installed.

find_package(CLI11 REQUIRED)

add_executable(vemesh_app
  ${CMAKE_SOURCE_DIR}/app/vemesh_app.cpp
  ${CMAKE_SOURCE_DIR}/app/vemesh_app_cli.cpp)

target_link_libraries(vemesh_app PRIVATE vemesh CLI11::CLI11)

# mesh generators that produce the test datasets
add_subdirectory(mesh_generators)

target_compile_features(vemesh_app PRIVATE cxx_std_17)

# stage the run script and the sample meshes next to the binary, so the script
# resolves all of its paths relative to its own location in the build tree.
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/scripts/run_sorgente.sh
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/sample_data
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
