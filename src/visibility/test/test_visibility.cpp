// Sriramajayam

#include <vm_visibility.h>
#include <vm_io.h>
#include <iostream>
#include <cassert>
#include <fstream>

int main()
{
  // environment
  const double coords[][2] =
    {{70, 76},
     {70, 74},
     {72, 74},
     {72.9206, 71.733},
     {73.9048, 74.0952},
     {73.9048, 76},
     {73.5246, 76.4162},
     {73.5724, 77.1908},
     {72.7337, 77.8255},
     {71.985, 76.9184},
     {70.0952, 77.9048}};
  const int nVertices = 11;
  
  // guard
  const double guard[] = {72, 76};
  
  std::fstream pfile;
  pfile.open("env.dat", std::ios::out);
  for(int n=0; n<nVertices; ++n)
    pfile << coords[n][0] << " " << coords[n][1] << std::endl;
  pfile << coords[0][0] << " " << coords[0][1] << std::endl;
  pfile << std::endl << guard[0] << " " << guard[1] << std::endl; 
  pfile.close();
  
  std::vector<pmp::Vertex> vertices{};
  pmp::SurfaceMesh mesh;
  for(int n=0; n<nVertices; ++n)
    vertices.push_back(mesh.add_vertex(pmp::Point(coords[n][0],coords[n][1],0.)));
  
  // guard location
  auto guard_vertex = mesh.add_vertex(pmp::Point(guard[0],guard[1],0.));
  
  // add faces
  mesh.add_face({guard_vertex, vertices[0], vertices[1], vertices[2]}); 
  mesh.add_face({guard_vertex, vertices[2], vertices[3], vertices[4]}); 
  mesh.add_face({guard_vertex, vertices[4], vertices[5]});
  mesh.add_face({guard_vertex, vertices[5], vertices[6], vertices[7]}); 
  mesh.add_face({guard_vertex, vertices[7], vertices[8], vertices[9]});
  mesh.add_face({guard_vertex, vertices[9], vertices[10]}); 
  mesh.add_face({guard_vertex, vertices[10], vertices[0]});
  vm::write_off(mesh, "mesh.off");
  vm::write_dat(mesh, "mesh.dat");
  
  // visibility polygon
  auto vis_poly_verts = vm::compute_visibility_polygon(mesh, guard_vertex);
  pfile.open("vis.dat", std::ios::out);
  for(auto& v:vis_poly_verts)
    pfile << v.first << " " << v.second << std::endl;
  pfile << vis_poly_verts.front().first << " " << vis_poly_verts.front().second << std::endl;
  pfile.close();
}

