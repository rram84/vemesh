# Sriramajayam

cmake_minimum_required(VERSION 3.12)

project(vemesh
  VERSION 1.0
  DESCRIPTION "VEMesh in 2D"
  LANGUAGES CXX)
 
# Set runtime path (for Linux)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Build options
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_TESTS "Run unit tests" OFF)  

# Dependencies
find_package(pmp REQUIRED)
find_package(eigen3 REQUIRED)
find_package(Boost REQUIRED)
if(BUILD_DOCS)
	find_package(Doxygen REQUIRED)
endif()

# Compile features
list(APPEND VM_COMPILE_FEATURES cxx_std_17)

# Compile code in src and test
add_subdirectory(src)

# Testing
if(${BUILD_TESTS})
	include(CTest)
	add_subdirectory(src/io/test)
	add_subdirectory(src/quality/test)
	add_subdirectory(src/optimizer/test)
	add_subdirectory(tutorial)
#	add_subdirectory(src/utils/test)
#	add_subdirectory(test/rect/test)
#	add_subdirectory(test/mesh-slicer/test)
endif()

# Install
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS
  vm_utils
  vm_io
  vm_quality
  vm_optimizer
  EXPORT ${PROJECT_NAME}Targets
  DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}Targets
  DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  NAMESPACE vm::
  FILE ${PROJECT_NAME}Targets.cmake)

set(VM_INCLUDE_DIRS_VAR ${PROJECT_NAME}/${CMAKE_INSTALL_INCLUDEDIR})
set(VM_LIBRARY_DIRS_VAR ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR})

configure_package_config_file(
  "cmake/${PROJECT_NAME}Config.cmake.in" "${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION
  ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  PATH_VARS VM_INCLUDE_DIRS_VAR VM_LIBRARY_DIRS_VAR)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PACKAGE_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION "${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")

if(BUILD_DOCS)
	add_subdirectory(docs)
endif()

