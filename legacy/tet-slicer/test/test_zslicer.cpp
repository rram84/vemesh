// Sriramajayam

#include <vm_TetMesh.h>
#include <iostream>
#include <vm_io.h>
#include <random>

int main()
{
  // generate a random z
  std::random_device rd; 
  std::mt19937 gen(rd()); 
  std::uniform_real_distribution<> dis(-0.9, 0.9);

  for(int i=0; i<25; ++i)
    {
      std::cout << std::endl << std::endl;
      
      // Read the tet mesh
      vm::TetMesh MD;
      MD.read_tec("cube.tec");
  
      // zcoord
      const double zcoord = dis(gen);
      std::cout << "Slicing at z-coordinate: " << zcoord << std::endl;
      
      // perturb nodes away from zcoord by a small tolerance 
      const double EPS    = 1.e-2;
      int num_vert_perturbed = MD.zperturb(zcoord, EPS);
      std::cout << "Perturbed " << num_vert_perturbed << " vertices away from the plane z = " << zcoord << std::endl;
      MD.write_tec("zslice-"+std::to_string(i) + ".tec");
      
      // slice the mesh at z = zcoord
      auto surf_mesh = MD.zslice(zcoord);
      vm::write_off(surf_mesh, "zslice-"+std::to_string(i)+".OFF");
    }

  // done
}
  
