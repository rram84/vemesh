// Sriramajayam

#include <vm_Manager.h>

int main()
{
  vm::Manager manager("coordinates.dat", "connectivity.dat");
  //vm::Manager manager("slice.OFF");
  manager.write("mesh.off");
  manager.write_bad_angles("bad_angles.off", 20.);

  // merge
  int nmerged = manager.merge_faces(20.);
  manager.inspect_mesh();
  std::cout << "Merged "<<nmerged << " elements "<< std::endl;
  manager.write("merged.off");
  
  // move vertices
  auto num_moved = manager.move_vertices(0.2, 10);
  manager.inspect_mesh();
  std::cout << "Moved " << num_moved.first << " vertices, unsuccessful at " << num_moved.second << std::endl;
  manager.write("moved.off");
  
  // snap
  /*manager.write_bad_vertices("bad_edges.off", 0.1);
  int nsnaps = manager.snap_vertices(0.1);
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
  manager.write("snapped-3.off");*/  
}
