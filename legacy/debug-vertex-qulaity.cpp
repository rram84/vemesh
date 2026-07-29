// Sriramajayam

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_vertex_ring.h>
#include <vm_inspect.h>
#include <vm_visibility.h>
#include <fstream>
#include <set>

int main()
{
  std::vector<pmp::Vertex> vertices{};
  pmp::SurfaceMesh mesh;
  vertices.push_back(mesh.add_vertex(pmp::Point(78., 58.,0.))); // 0
  vertices.push_back(mesh.add_vertex(pmp::Point(78.0241, 57.8268,0.))); // 1
  vertices.push_back(mesh.add_vertex(pmp::Point(78.0154, 55.9846,0.))); // 2
  vertices.push_back(mesh.add_vertex(pmp::Point(78.133, 57.6889,0.))); // 3
  vertices.push_back(mesh.add_vertex(pmp::Point(78.6506, 56.2453,0.))); // 4
  vertices.push_back(mesh.add_vertex(pmp::Point(78.858, 56.65,0.))); // 5
  vertices.push_back(mesh.add_vertex(pmp::Point(79.4392, 56.5608,0.))); // 6
  vertices.push_back(mesh.add_vertex(pmp::Point(80.0363, 57.732,0.))); // 7
  vertices.push_back(mesh.add_vertex(pmp::Point(80.1221, 57.9,0.))); // 8
  vertices.push_back(mesh.add_vertex(pmp::Point(79.4961, 57.8869,0.))); // 9
  vertices.push_back(mesh.add_vertex(pmp::Point(79.172, 58.7818,0.))); // 10
  vertices.push_back(mesh.add_vertex(pmp::Point(79.1624, 58.02,0.))); // 11
  vertices.push_back(mesh.add_vertex(pmp::Point(78.6899, 56.934,0.))); // 12 - inner vertex
  //vertices.push_back(mesh.add_vertex(pmp::Point(78.6899, 56.934, 0.))); // 12 - inner vertex

  mesh.add_face({vertices[0],vertices[1],vertices[12],vertices[11]});
  mesh.add_face({vertices[1],vertices[2],vertices[3],vertices[12]});
  mesh.add_face({vertices[3],vertices[4],vertices[5],vertices[12]});
  mesh.add_face({vertices[5],vertices[6],vertices[7],vertices[8],vertices[9],vertices[12]});
  mesh.add_face({vertices[9],vertices[10],vertices[11],vertices[12]});
  
  vm::write_off(mesh, "mesh.off");
  vm::write_dat(mesh, "mesh.dat");

  // quality
  vm::LimitCircle_t lc = vm::compute_distance_based_vertex_quality(mesh, vertices[12]);
  std::cout << "Center: " << lc.center[0] << " " << lc.center[1] << ", " << "radius: " << lc.radius << std::endl;
}
