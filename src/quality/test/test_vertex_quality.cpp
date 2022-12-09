// Sriramajayam

#include <vm_quality.h>
#include <iostream>
#include <fstream>

int main()
{
  std::vector<pmp::Vertex> vertices{};
  pmp::SurfaceMesh mesh;
  
  // outer ring coordinates
  const double coords[][2] = {{2.0, 1.3}, {2.4, 1.7}, {2.8, 1.8}, {3.7, 1.6},
  			      {3.4, 2.0}, {4.1, 3.0}, {5.3, 2.6}, {5.4, 1.2}, {4.9, 0.8}, {2.9, 0.7}};
  for(int n=0; n<10; ++n)
    vertices.push_back(mesh.add_vertex(pmp::Point(coords[n][0],coords[n][1],0.)));
  
  // inner vertex
  const double vert[] = {2.81,1.35};
  auto inner_vertex = mesh.add_vertex(pmp::Point(vert[0],vert[1],0.));

  // add faces
  mesh.add_face({inner_vertex, vertices[0], vertices[9]});
  mesh.add_face({inner_vertex, vertices[9], vertices[8]});
  mesh.add_face({inner_vertex, vertices[8], vertices[7], vertices[6], vertices[5], vertices[4], vertices[3], vertices[2], vertices[1], vertices[0]});

  // visualize
  std::fstream pfile;
  pfile.open("mesh.dat", std::ios::out);
  assert(pfile.good());
  auto faces = mesh.faces();
  for(auto face:faces)
    {
      auto v_circulator = mesh.vertices(face);
      for(auto v:v_circulator)
	{
	  const auto& X = mesh.position(v);
	  pfile << X[0] <<" " << X[1] << std::endl;
	}
      for(auto v:v_circulator)
	{
	  const auto& X = mesh.position(v);
	  pfile << X[0] <<" " << X[1] << std::endl;
	  break;
	}
      pfile << std::endl << std::endl;
    }
  pfile.close();


  // check the vertex quality
  auto q = vm::vertex_quality(mesh, inner_vertex);
  std::cout << "Edge ratio and min angle: " << q.first << ", " << q.second << std::endl;
}
