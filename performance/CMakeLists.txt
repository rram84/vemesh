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
