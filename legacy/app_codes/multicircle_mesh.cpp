// Sriramajayam

#include <vm_test_mesh_slicer.h>
#include <vm_io.h>
#include <vm_utils.h>
#include <vm_Manager.h>
#include <vm_quality.h>
#include <filesystem>
#include <cmath>
#include <random>

// signed distance function to a circle centered at the origin of a given radius
double multicirc_signed_distance(const double* X, const std::vector<std::vector<double>>& centers, const double rad) {

  // closest circle
  double min_dist = 1.e6;
  int min_idx = -1;
  const int ncircles = static_cast<int>(centers.size());
  for(int i=0; i<ncircles; ++i) {
    const auto& P = centers[i];
    double dist = std::abs(std::sqrt((X[0]-P[0])*(X[0]-P[0])+(X[1]-P[1])*(X[1]-P[1]))-rad);
    if(dist<min_dist) {
      min_dist = dist;
      min_idx = i;
    }
  }
  assert(min_idx!=-1);
  const auto& P = centers[min_idx];      
  return std::sqrt((X[0]-P[0])*(X[0]-P[0])+(X[1]-P[1])*(X[1]-P[1]))-rad;
}


int main() {

  // generate 10 circle centers of radius 0.1
  const double crad = 0.1;
  std::random_device rd;
  std::mt19937 gen(rd()); 
  std::uniform_real_distribution<double> xdist(-0.4,0.4);
  std::uniform_real_distribution<double> ydist(-0.9,0.9);
  std::vector<std::vector<double>> centers{};
  int ncircles = 0;
  while(ncircles<10) {
    std::cout << "Generating circle #" << ncircles << std::endl;
    
    // generate a random center for this circle
    double P[] = {xdist(gen), ydist(gen)};

    // should be away from the boundaries
    bool flag = true;
    flag = std::abs(std::abs(P[0])-0.5)>1.1*crad && std::abs(std::abs(P[1])-1.0)>1.1*crad;
    
    // should be away from all previous circles
    for(int i=0; i<ncircles && flag==true; ++i) {
      const auto& Q = centers[i];
      double dist = std::sqrt((P[0]-Q[0])*(P[0]-Q[0])+(P[1]-Q[1])*(P[1]-Q[1]));
      if(dist<2.5*crad)
	flag = false;
    }

    if(flag==true) {
      centers.push_back({P[0],P[1]});
      ++ncircles;
      std::cout << "generated circle #" << ncircles;
    }
  }

  std::string in_meshfile = "Rect.3.off";
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
  vm::test::LevelSetFunction_t sdfunc = std::bind(multicirc_signed_distance, std::placeholders::_1, centers, crad);
    
  // adjust node positions away from the zero level sets
  const double sdtol = 1.e-6;
  vm::test::adjust_mesh_nodes(mesh, sdtol, 1.1*sdtol, sdfunc);
  bool is_mesh_ok = vm::inspect_mesh(mesh);
  assert(is_mesh_ok==true);
    
  // embed the inner boundary
  vm::test::embed_interface(mesh, sdtol, sdfunc, {2,1}, 2); // shifting tolerance, {mat1, mat2}, interface node id
  assert(vm::inspect_mesh(mesh));
    
  // save
  vm::write_vtk(mesh, "embed.vtk");
  vm::write_suku_format(mesh, "embed");

  // Mesh manager
  vm::Manager manager("embed.vtk");
  manager.merge_faces(vm::FaceQuality::stiffness, 0.2, 1.2, nullptr);
  vm::write_vtk(manager.get_mesh(), "embed-a.vtk");
  vm::write_suku_format(manager.get_mesh(), "embed-a");
}
