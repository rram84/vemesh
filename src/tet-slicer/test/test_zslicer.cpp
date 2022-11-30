// Sriramajayam

#include <vm_TetMesh.h>
#include <iostream>
#include <vm_io.h>

int main()
{
  
  // Read the tet mesh
  vm::TetMesh MD;
  MD.read_tec("tetmesh.tec");

  // perturb nodes away from z = 50
  const double zcoord = 50.;
  const double EPS    = 1.e-1;
  int num_vert_perturbed = MD.zperturb(zcoord, EPS);
  std::cout << "Perturbed " << num_vert_perturbed << " vertices away from the plane z = " << zcoord << std::endl;
  MD.write_tec("zpert.tec");
  
  // slice the mesh at z = zcoord
  auto surf_mesh = MD.zslice(zcoord);
  vm::write_off(surf_mesh, "slice.OFF");
}
  
