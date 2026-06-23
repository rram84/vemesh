// Sriramajayam

#include <vm_test_mesh_slicer.h>
#include <vm_io.h>
#include <vm_utils.h>
#include <vm_Manager.h>
#include <vm_quality.h>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <random>

// signed distance function to a circle centered at the origin of a given radius
double circ_signed_distance(const double* X, const double rad, const double sign) {
  return sign*(std::sqrt(X[0]*X[0]+X[1]*X[1])-rad);
}

int main() {
  {
    vm::Manager manager("17333/embed-a-667.vtk");
    manager.compute_face_qualities(vm::FaceQuality::stiffness);
    auto mesh= manager.get_mesh();
    auto fqualities = mesh.get_face_property<double>("face_quality");
    auto faces = mesh.faces();
    std::vector<double> qvals{};
    for(auto f:faces)
      qvals.push_back(fqualities[f]);
    std::sort(qvals.begin(), qvals.end());
    const int N = static_cast<int>(qvals.size());
    std::fstream file;
    file.open("post-17333.dat", std::ios::out);
    for(int i=0; i<N; ++i)
      file << i+1 << " " << qvals[i] << std::endl;
    file.close();
    exit(1);
  }
  
  std::string in_meshfile = "Rect.4.off";
  const double hval= 0.15/8.;
  
  // read the input mesh
  pmp::SurfaceMesh mesh;
  vm::read_off(in_meshfile, mesh);
  
  // center the input mesh at the origin
  auto& coords = mesh.positions();
  for(auto& X:coords) {
    X[0] -= 0.5;
    X[1] -= 1.0;
  }
  
  // create the signed distance functions
  vm::test::LevelSetFunction_t sdfunc = std::bind(circ_signed_distance, std::placeholders::_1, 0.4, 1.0);

  // identify mesh nodes near the circular interface
  std::vector<pmp::Vertex> proximal_vertices{};
  auto v_container = mesh.vertices();
  for(auto v:v_container)
    if(mesh.is_boundary(v)==false) {
      auto& X = mesh.position(v);
      double Y[] = {X[0],X[1]};
      double sdval = sdfunc(Y);
      if(std::abs(sdval)<1.25*hval)
	proximal_vertices.push_back(v);
    }

  // Create a random number generator
  std::random_device rd;
  std::mt19937 generator(rd()); 
  std::uniform_real_distribution<double> distribution(-0.15*hval, 0.15*hval);

  for(int iter=750; iter<1000; ++iter) {
    std::cout << "Realization: " << iter << std::endl;

    // perturbed mesh
    pmp::SurfaceMesh pert_mesh = mesh;
    for(auto& v:proximal_vertices) {
      auto& X = pert_mesh.position(v);
      X[0] += distribution(generator);
      X[1] += distribution(generator);
    }
    
    // adjust node positions away from the zero level sets
    const double sdtol = 1.e-6;
    vm::test::adjust_mesh_nodes(pert_mesh, sdtol, 1.1*sdtol, sdfunc);
    bool is_mesh_ok = vm::inspect_mesh(pert_mesh);
    if(is_mesh_ok==false) {
      std::cout << "generated an invalid mesh" << std::endl;
      continue;
    }
    //vm::write_vtk(pert_mesh, "h4/pert-"+std::to_string(iter)+".vtk");
    
    // embed the inner boundary
    vm::test::embed_interface(pert_mesh, sdtol, sdfunc, {2,1}, 2); // shifting tolerance, {mat1, mat2}, interface node id
    assert(vm::inspect_mesh(pert_mesh));
    
    // save
    vm::write_vtk(pert_mesh, "h4/embed-"+std::to_string(iter)+".vtk");
    vm::write_suku_format(pert_mesh, "h4/embed-"+std::to_string(iter));

    // Mesh manager
    vm::Manager manager("h4/embed-"+std::to_string(iter)+".vtk");
    manager.merge_faces(vm::FaceQuality::stiffness, 0.2, 1.2, nullptr);
    vm::write_vtk(manager.get_mesh(), "h4/embed-a-"+std::to_string(iter)+".vtk");
    vm::write_suku_format(manager.get_mesh(), "h4/embed-a-"+std::to_string(iter));
  }
  
  // done
}
