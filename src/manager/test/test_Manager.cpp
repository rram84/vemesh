// Sriramajayam

#include <vm_manager.h>

int main()
{
  vm::Manager manager("coordinates.dat", "connectivity.dat");
  manager.write("mesh.off");
  manager.write_bad_angles("bad_angles.off", 20.);
  manager.write_bad_vertices("bad_edges.off", 0.1);

  // merge
  manager.merge(20.);
  manager.write("merged.off");
  manager.write_bad_vertices("merged_bad_edges.off", 0.1);
    
}
