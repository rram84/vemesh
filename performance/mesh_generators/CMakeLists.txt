# Sriramajayam
#
# Generators that produce reproducible mesh datasets for the performance tests.

set(TARGETS
  generate_embedded_meshes
  generate_clipped_meshes)

foreach(TGT IN LISTS TARGETS)
  add_executable(${TGT} ${TGT}.cpp)
  target_link_libraries(${TGT} PRIVATE
    vm_io
    vm_utils
    vm_tutorial_utils
    CLI11::CLI11)
endforeach()
