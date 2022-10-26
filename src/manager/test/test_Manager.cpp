// Sriramajayam

#include <vm_Manager.h>

int main()
{
  vm::Manager manager("coordinates.dat", "connectivity.dat");
  manager.write("mesh.off");
  manager.write_bad_angles("bad_angles.off", 20.);
  manager.write_bad_vertices("bad_edges.off", 0.1);
  
  // merge
  int nmerged = manager.merge(20.);
  manager.inspect_mesh();
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged-1.off");

  // merge again
  nmerged = manager.merge(20.);
  manager.inspect_mesh();
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged-2.off");

  // snap
  int nsnaps = manager.snap(0.2);
  manager.inspect_mesh();
  std::cout << "Snapped " << nsnaps << " vertices " << std::endl;
  manager.write("snapped-1.off");

  nsnaps = manager.snap(0.2);
  manager.inspect_mesh();
  std::cout << "Snapped " << nsnaps << " vertices " << std::endl;
  manager.write("snapped-2.off");

  nsnaps = manager.snap(0.25);
  manager.inspect_mesh();
  std::cout << "Snapped " << nsnaps << " vertices " << std::endl;
  manager.write("snapped-3.off");
}
