// Sriramajayam

#include <vm_TetMesh.h>
#include <fstream>
#include <iostream>

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
  std::vector<std::array<double,2>> intersect_coords{};
  std::vector<std::vector<int>> intersect_conn{};
  MD.zslice(zcoord, intersect_coords, intersect_conn);

  int nTri = 0;
  int nQuad = 0;
  for(auto& conn:intersect_conn)
    if(conn.size()==3)
      ++nTri;
    else
      ++nQuad;
  
  // write coordinates and connectivity as an OFF file
  std::fstream pfile;
  pfile.open("tri.OFF", std::ios::out);
  pfile << "OFF" << std::endl << intersect_coords.size() << " " << nTri << " " << 0;
  for(auto& X:intersect_coords)
    pfile << std::endl << X[0] <<", " << X[1] << ", " << zcoord;
  for(auto& conn:intersect_conn)
    if(conn.size()==3)
      {
	pfile << std::endl << conn.size() << " ";
	for(auto& n:conn)
	  pfile << n << " ";
      }
  pfile.close();

  pfile.open("quad.OFF", std::ios::out);
  pfile << "OFF" << std::endl << intersect_coords.size() << " " << nQuad << " " << 0;
  for(auto& X:intersect_coords)
    pfile << std::endl << X[0] <<", " << X[1] << ", " << zcoord;
  for(auto& conn:intersect_conn)
    if(conn.size()==4)
      {
	pfile << std::endl << conn.size() << " ";
	for(auto& n:conn)
	  pfile << n << " ";
      }
  pfile.close();
  
}
  
