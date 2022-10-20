// Sriramajayam

#include <vm_manager.h>

int main()
{
  vm::Manager manager("coordinates.dat", "connectivity.dat");
  manager.write("mesh.off");
  manager.write_bad_angles("bad_angles.off", 20.);
  manager.write_bad_vertices("bad_edges.off", 0.1);

  // merge
  int nmerged = manager.merge(20.);
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged-1.off");
  
  // merge again
  nmerged = manager.merge(20.);
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged-2.off");

  nmerged = manager.merge(20.);
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged-3.off");
  
  nmerged = manager.merge(20.);
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged-4.off");
  
}
