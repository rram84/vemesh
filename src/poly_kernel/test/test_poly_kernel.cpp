// Sriramajayam

#include <vm_polygon_kernel.h>
#include <fstream>

int main()
{
  // Create a mesh with one pentagonal polygon
  //const double coords[] = {{0.,0.}, {2.,0.}, {2.,2.}, {1.,1.}, {0.,2.}};
  const double coords[][2] = {{2.0, 1.3}, {2.4, 1.7}, {2.8, 1.8}, {3.7, 1.6},
			      {3.4, 2.0}, {4.1, 3.0}, {5.3, 2.6}, {5.4, 1.2}, {4.9, 0.8}, {2.9, 0.7}};
  const int nVerts = 10;
  
  pmp::SurfaceMesh mesh;
  std::vector<pmp::Vertex> vertices{};
  for(int n=nVerts-1; n>=0; --n)
    vertices.push_back( mesh.add_vertex(pmp::Point(coords[n][0], coords[n][1], 0.)) );
  auto face = mesh.add_face(vertices);

  // polygon vertices
  std::fstream pfile;
  pfile.open("polygon.dat", std::ios::out);
  assert(pfile.good());
  for(int n=0; n<nVerts; ++n)
    pfile << coords[n][0] <<" " << coords[n][1] << std::endl;
  pfile << coords[0][0] << " " << coords[0][1] << std::endl;   // repeat the last vertex to close the loop
  pfile.close();
  
  // kernel
  auto kernel_verts = vm::compute_polygon_kernel(mesh, face);
  pfile.open("kernel.dat", std::ios::out);
  assert(pfile.good());
  for(auto& v:kernel_verts)
    pfile << v.first <<" " << v.second << std::endl;
  // repeat the first vertex
  for(auto& v:kernel_verts)
    {
      pfile << v.first << " " << v.second << std::endl;
      break;
    }
  pfile.close();

}
