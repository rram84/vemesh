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

add_executable(vemesh_app vemesh_app.cpp)

target_link_libraries(vemesh_app
  PRIVATE
  vemesh::vemesh
  CLI11::CLI11)

target_compile_features(vemesh_app PRIVATE cxx_std_17)
