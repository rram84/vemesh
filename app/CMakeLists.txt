# Sriramajayam

# Build the vemesh application

cmake_minimum_required(VERSION 3.17)

project(vemesh_app
  DESCRIPTION "Command-line driver for vemesh"
  LANGUAGES CXX)

# find vemesh
find_package(vemesh REQUIRED)

# Optional OpenMP: lets vemesh_app report the thread count in quality/inspection loops
 find_package(OpenMP)

add_executable(vemesh_app
			vemesh_app.cpp
			vemesh_app_cli.cpp)

target_link_libraries(vemesh_app
  PRIVATE
  vemesh::vemesh)

# Command-line parsing uses the single-header CLI11 provided in ../external,
# included as <CLI11.hpp> by vemesh_app_cli.cpp — no external CLI11 dependency.
target_include_directories(vemesh_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../external)

if(OpenMP_FOUND)
  target_link_libraries(vemesh_app PRIVATE OpenMP::OpenMP_CXX)
endif()

target_compile_features(vemesh_app PRIVATE cxx_std_17)
