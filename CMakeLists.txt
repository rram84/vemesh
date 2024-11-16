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

# Compile code in src and test
add_subdirectory(src)
add_subdirectory(test)

# Install
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS
  vm_inspect
  vm_io
  vm_manager
  vm_merge
  vm_move
  vm_quality
  vm_stiffness
  vm_vertex_ring
  vm_visibility
  vm_test_rect
  vm_test_mesh_slicer
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

# Testing
include(CTest)
add_subdirectory(src/manager/test)
add_subdirectory(src/visibility/test)
add_subdirectory(src/quality/test)
add_subdirectory(src/move/test)	
add_subdirectory(src/stiffness/test)
add_subdirectory(app)

add_subdirectory(test/rect/test)
add_subdirectory(test/mesh-slicer/test)

#add_subdirectory(cmame-paper/)
#add_subdirectory(ppt/)



