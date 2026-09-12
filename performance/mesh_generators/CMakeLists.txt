# Sriramajayam
#
# Generators that produce reproducible mesh datasets for the performance tests.

set(TARGETS
  embed_shapes
  clip_circle)

foreach(TGT IN LISTS TARGETS)
  add_executable(${TGT} ${TGT}.cpp)
  # vm_utils comes in transitively via vm_tutorial_utils (PUBLIC); listing it
  # again here puts the static lib on the link line twice (ld duplicate warning).
  target_link_libraries(${TGT} PRIVATE
    vm_io
    vm_tutorial_utils
    vemesh_cli11)
endforeach()
