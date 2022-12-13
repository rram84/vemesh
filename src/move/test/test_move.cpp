// Sriramajayam

#include <vm_move.h>
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
  const double guard[] = {2.88,0.85};
  
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

  // halfedges at the vertex
  {
    pfile.open("in-edges.dat", std::ios::out);
    auto h_circulator = mesh.halfedges(guard_vertex);
    for(auto h:h_circulator)
      {
	const auto& X = mesh.position(mesh.from_vertex(h));
	const auto& Y = mesh.position(mesh.to_vertex(h));
	pfile << X[0] << " " << X[1] << std::endl
	      << Y[0] << " " << Y[1] << std::endl << std::endl;
      }
    pfile.close();
  }
  
  // compute a feasible location
  auto move_result = vm::compute_feasible_vertex_position(mesh, guard_vertex, 0.2, 20.0, 20);  // edge ratio, angle tolerance, num_samples
  if(move_result.first==true)
    {
      mesh.position(guard_vertex) = pmp::Point(move_result.second.first, move_result.second.second, 0.);
      pfile.open("out-edges.dat", std::ios::out);
      auto h_circulator = mesh.halfedges(guard_vertex);
      for(auto h:h_circulator)
	{
	  const auto& X = mesh.position(mesh.from_vertex(h));
	  const auto& Y = mesh.position(mesh.to_vertex(h));
	  pfile << X[0] << " " << X[1] << std::endl
		<< Y[0] << " " << Y[1] << std::endl << std::endl;
	}
      pfile.close();  
    }
  
}

