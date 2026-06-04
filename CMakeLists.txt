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

# Build options
option(BUILD_DOCS "Build documentation" OFF)
option(BUILD_TESTS "Build unit tests" ON)  

# Dependencies
find_package(Eigen3 REQUIRED)
find_package(Boost 1.87 REQUIRED CONFIG)
find_package(OpenMP)                        # optional
if(OpenMP_FOUND)
  message(STATUS "vemesh: OpenMP found (${OpenMP_CXX_VERSION}) - quality-evaluation loops will be parallelized")
else()
  message(STATUS "vemesh: OpenMP NOT found - quality-evaluation loops will run serially")
endif()
find_package(pmp 3.0 REQUIRED)
if(BUILD_DOCS)
	find_package(Doxygen REQUIRED)
endif()

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
  add_subdirectory(tutorial)
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

