// Sriramajayam

#include <vm_quality.h>
#include <iostream>
#include <fstream>

int main()
{
  std::vector<pmp::Vertex> vertices{};
  pmp::SurfaceMesh mesh;

  // create a triangle
  vertices.push_back(mesh.add_vertex(pmp::Point({0.,0.,0.})));
  vertices.push_back(mesh.add_vertex(pmp::Point({1.,0.,0.})));
  vertices.push_back(mesh.add_vertex(pmp::Point({0.,1.,0.})));
  auto tri = mesh.add_face(vertices);

  // compute face quality
  double quality = vm::FaceQuality::stiffness(mesh, tri);
  std::cout << std::endl << "Triangle quality: " << quality << std::endl;
  
}
