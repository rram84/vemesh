// Sriramajayam

#include <vm_manager.h>

int main()
{
  vm::Manager manager("coordinates.dat", "connectivity.dat");
  manager.write("mesh.off");
  manager.write_bad_angles("bad_angles.off", 10.);
  manager.write_bad_vertices("bad_edges.off", 0.1);
}
