# Sriramajayam

# Build the vemesh application

cmake_minimum_required(VERSION 3.17)

project(vemesh_app
  DESCRIPTION "Command-line driver for vemesh"
  LANGUAGES CXX)

# find vemesh
find_package(vemesh REQUIRED)

# CLI11 for command-line parsing
find_package(CLI11 REQUIRED)

# Optional OpenMP: lets vemesh_app report the thread count in quality/inspection loops
 find_package(OpenMP)

add_executable(vemesh_app
			vemesh_app.cpp
			vemesh_app_cli.cpp)

target_link_libraries(vemesh_app
  PRIVATE
  vemesh::vemesh
  CLI11::CLI11)

if(OpenMP_FOUND)
  target_link_libraries(vemesh_app PRIVATE OpenMP::OpenMP_CXX)
endif()

target_compile_features(vemesh_app PRIVATE cxx_std_17)
