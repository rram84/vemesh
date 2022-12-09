# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(vemesh
  VERSION 1.0
  DESCRIPTION "Universal Meshes in 2D"
  LANGUAGES CXX)

# Set runtime path (for Linux)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Dependencies
find_package(pmp REQUIRED)
find_package(eigen3 REQUIRED)
find_package(CLI11 REQUIRED)
find_package(Boost REQUIRED)
find_package(CGAL REQUIRED)

list(APPEND VM_COMPILE_FEATURES cxx_std_17)

# Compile code in src and mesh
add_subdirectory(src)

# Testing
include(CTest)
add_subdirectory(src/manager/test)
add_subdirectory(src/visibility/test)
add_subdirectory(src/sampling/test)
add_subdirectory(src/quality/test)
add_subdirectory(src/tet-slicer/test)



