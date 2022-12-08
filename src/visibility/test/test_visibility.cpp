// Sriramajayam

#include <vm_visibility.h>
#include <vm_io.h>
#include <iostream>
#include <cassert>
#include <fstream>

int main()
{
  // environment
  const double coords[][2] = {{2.0, 1.3}, {2.4, 1.7}, {2.8, 1.8}, {3.7, 1.6},
  			      {3.4, 2.0}, {4.1, 3.0}, {5.3, 2.6}, {5.4, 1.2}, {4.9, 0.8}, {2.9, 0.7}};
  const int nVertices = 10;

  // guard
  const double guard[] = {2.81,1.35};
  
  std::fstream pfile;
  pfile.open("env.dat", std::ios::out);
  for(int n=0; n<10; ++n)
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
  mesh.add_face({guard_vertex, vertices[0], vertices[9]});
  mesh.add_face({guard_vertex, vertices[9], vertices[8]});
  mesh.add_face({guard_vertex, vertices[8], vertices[7], vertices[6], vertices[5], vertices[4], vertices[3], vertices[2], vertices[1], vertices[0]});
  vm::write_off(mesh, "mesh.off");
  
  // visibility polygon
  vm::compute_visibility_polygon(mesh, guard_vertex);
}

