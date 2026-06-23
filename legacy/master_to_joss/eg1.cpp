// Sriramajayam

#include <vm_quality.h>
#include <iostream>
#include <fstream>

int main()
{
  std::fstream file;
  file.open("K12.dat", std::ios::out);
  for(int i=1; i<500000; i+=100)
    {
      float eps = static_cast<double>(i)*(1.e-6);
      pmp::SurfaceMesh mesh;
      std::vector<pmp::Vertex> vertices{};
      vertices.push_back(mesh.add_vertex(pmp::Point{0.,0.,0.}));
      vertices.push_back(mesh.add_vertex(pmp::Point{1,0.,0.}));
      vertices.push_back(mesh.add_vertex(pmp::Point{0.5,eps,0.}));
      vertices.push_back(mesh.add_vertex(pmp::Point{0.,1.,0.}));

      //auto K1 = mesh.add_face({vertices[0],vertices[1],vertices[2]});
      //double sigma = vm::MeshFaceQuality_f(mesh, K1, vm::FaceQuality::stiffness);
      
      //auto K2 = mesh.add_face({vertices[0],vertices[2],vertices[3]});
      //double sigma = vm::MeshFaceQuality_f(mesh, K2, vm::FaceQuality::stiffness);
      
      auto K12 = mesh.add_face({vertices[0],vertices[1],vertices[2],vertices[3]});
      double sigma = vm::MeshFaceQuality_f(mesh, K12, vm::FaceQuality::stiffness);
      
      file << eps << " " << sigma << std::endl;
    }
  file.close();
      
}
  
