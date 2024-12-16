// Sriramajayam

#include <vm_test_mesh_slicer.h>
#include <vm_io.h>
#include <vm_utils.h>
#include <CLI/CLI.hpp>
#include <filesystem>
#include <cmath>

// Options
// -i input mesh file in off or vtk file format
// -o output mesh file in off or vtk file format

// signed distance function to a circle centered at the origin of a given radius
double circ_signed_distance(const double* X, const double rad, const double sign) {
  return sign*(std::sqrt(X[0]*X[0]+X[1]*X[1])-rad);
}


int main(int argc, char** argv) {

  // Command line options
  CLI::App app;
  app.footer("Create an annular mesh\n \
              Sample usage: ./annulus_mesh -i mesh.off -o mesh-cut.off");

  // options
  std::string in_meshfile, out_meshfile;
  app.add_option("-i", in_meshfile, "input mesh file in off or vtk format")->required()->check(CLI::ExistingFile);
  app.add_option("-o", out_meshfile, "output mesh file in off or vtk format")->required();

  // parse
  CLI11_PARSE(app, argc, argv);
  
  // read the input mesh
  std::string ext = std::filesystem::path(in_meshfile).extension();
  assert(ext==".off" || ext==".OFF" || ext==".vtk");
  pmp::SurfaceMesh mesh;
  if(ext==".off" || ext==".OFF")
    vm::read_off(in_meshfile, mesh);
  else
    vm::read_vtk(in_meshfile, mesh);

  // scale the input mesh to size sqrt(5)
  /*const double factor = std::sqrt(5.)/2;
    auto& coords = mesh.positions();
    for(auto& X:coords)
    for(int k=0; k<3; ++k)
    X[k] *= factor;
    vm::inspect_mesh(mesh);*/
    
  // scale the input mesh to size sqrt(5)
  const double factor = std::sqrt(5.);
  auto& coords = mesh.positions();
  for(auto& X:coords) {
    X[0] -= 0.5;
    X[1] -= 1.0;
    X[0] *= factor;
    X[1] *= factor;
  }
  
  // create the signed distance functions
  vm::test::LevelSetFunction_t out_sdfunc = std::bind(circ_signed_distance, std::placeholders::_1, 1.0, 1.0);
  vm::test::LevelSetFunction_t in_sdfunc = std::bind(circ_signed_distance, std::placeholders::_1, 0.4, -1.0);

  // adjust node positions away from the zero level sets
  const double sdtol = 1.e-8;
  vm::test::adjust_mesh_nodes(mesh, sdtol, 1.1*sdtol, out_sdfunc);
  vm::test::adjust_mesh_nodes(mesh, sdtol, 1.1*sdtol, in_sdfunc);
  
  // clip with the outer boundary
  vm::test::clip_mesh(mesh, sdtol, out_sdfunc, 1, 3); // shifting tolerance, material id, boundary node id
  vm::write_vtk(mesh, out_meshfile);
  vm::inspect_mesh(mesh); 
  
  // embed the inner boundary
  vm::test::embed_interface(mesh, sdtol, in_sdfunc, {2,1}, 2); // shifting tolerance, {mat1, mat2}, interface node id
  vm::inspect_mesh(mesh);
  
  // save
  ext = std::filesystem::path(out_meshfile).extension();
  assert(ext==".off" || ext==".OFF" || ext==".vtk");
  if(ext==".off" || ext==".OFF")
    vm::write_off(mesh, out_meshfile);
  else
    vm::write_vtk(mesh, out_meshfile);

  // done
}
