# Sriramajayam

cmake_minimum_required(VERSION 3.17)

project(vemesh
  VERSION 1.0
  DESCRIPTION "vemesh in 2D"
  LANGUAGES CXX)
 
# Set runtime path (for Linux)
include(GNUInstallDirs)
if(UNIX AND NOT APPLE)
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# Default to an optimised build when the user has not specified one.
# (Skip for multi-config generators, which pick the type at build time.)
get_property(_vm_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _vm_multi_config AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
    "Build type (Release, Debug, RelWithDebInfo, MinSizeRel)" FORCE)
  message(STATUS "vemesh: CMAKE_BUILD_TYPE not set, defaulting to Release")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    Debug Release RelWithDebInfo MinSizeRel)
endif()

# Build options
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_TESTS "Build unit tests" ON)

message(STATUS "vemesh build options (override with -D<option>=ON/OFF):")
message(STATUS "  CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}  (Debug | Release | RelWithDebInfo)")
message(STATUS "  BUILD_TESTS      = ${BUILD_TESTS}  (build unit tests)")
message(STATUS "  BUILD_DOCS       = ${BUILD_DOCS}  (build Doxygen documentation)")

# Dependencies
find_package(Eigen3 REQUIRED)
find_package(Boost 1.87 REQUIRED CONFIG)
find_package(OpenMP)                        # optional
find_package(pmp REQUIRED)
if(BUILD_DOCS)
	find_package(Doxygen REQUIRED)
endif()

# Report found dependencies
message(STATUS "vemesh dependencies:")
message(STATUS "  Eigen3 ${Eigen3_VERSION} (${Eigen3_DIR})")
message(STATUS "  Boost  ${Boost_VERSION} (${Boost_INCLUDE_DIRS})")
message(STATUS "  PMP    ${pmp_VERSION} (${pmp_DIR})")
if(OpenMP_FOUND)
  message(STATUS "  OpenMP ${OpenMP_CXX_VERSION}")
else()
  message(STATUS "  OpenMP not found (optional) - vemesh will run serially")
endif()

# Third-party headers provided in-tree (defines the vemesh_cli11 interface
# target, consumed by the in-tree command-line frontends).
add_subdirectory(external)

# Compile features
list(APPEND VM_COMPILE_FEATURES cxx_std_17)

# Compile code in src and test
add_subdirectory(src)

add_library(vemesh INTERFACE)

target_link_libraries(vemesh
    INTERFACE
    vemesh::vm_utils
    vemesh::vm_io
    vemesh::vm_quality
    vemesh::vm_optimizer)

target_compile_features(vemesh INTERFACE ${VM_COMPILE_FEATURES})

# Testing
if(BUILD_TESTS)
  include(CTest)
  add_subdirectory(src/io/test)
  add_subdirectory(src/utils/test)
  add_subdirectory(src/quality/test)
  add_subdirectory(src/optimizer/test)

  # single copy of the sample meshes in the build tree; the tutorial and
  # performance subdirs symlink to this rather than each keeping its own copy.
  file(COPY ${CMAKE_SOURCE_DIR}/sample_data DESTINATION ${CMAKE_BINARY_DIR})

  # shared tutorial-utilities library, reused by the tutorial examples and the performance test generators
  add_subdirectory(tutorial/common)
  add_subdirectory(tutorial)

  # in-tree performance tooling
  add_subdirectory(performance)
endif()

# Install
include(CMakePackageConfigHelpers)

install(TARGETS
  vm_utils
  vm_io
  vm_quality
  vm_optimizer
  EXPORT ${PROJECT_NAME}Targets
  DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS vemesh EXPORT ${PROJECT_NAME}Targets)

install(EXPORT ${PROJECT_NAME}Targets
  DESTINATION ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  NAMESPACE vemesh::
  FILE ${PROJECT_NAME}Targets.cmake)

configure_package_config_file(
  "cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION
    ${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
    
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION "${PROJECT_NAME}/${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE.txt")

if(BUILD_DOCS)
	add_subdirectory(docs)
endif()

