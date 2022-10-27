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
find_package(eigen3 REQUIRED)
find_package(Boost REQUIRED)

# external packages: CLI11 & PMP
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(vemesh_DownloadExternal)

# Find CLI11
find_package(CLI11 QUIET)
if(NOT CLI11_FOUND)
	message(STATUS "Could not find CLI11. Downloading.")
	vemesh_download_cli11(${CMAKE_BINARY_DIR}/extern/cli11)
else()
	message(STATUS "Found CLI11")
endif()


# Find PMP library
find_package(pmp QUIET)
if(NOT pmp_FOUND) 
       message(STATUS "Could not find pmp. Downloading.")
       vemesh_download_pmp(${CMAKE_BINARY_DIR}/extern/pmp)
else()
	message(STATUS "Found pmp")
endif()



# compiler options
list(APPEND VM_COMPILE_FEATURES cxx_std_14)

# Compile code in src and mesh
add_subdirectory(src)

# Testing
include(CTest)
add_subdirectory(src/manager/test)


