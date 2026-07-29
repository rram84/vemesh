// Sriramajayam

// Clip a given mesh with circle with specifid center and radius

// Option
// -i Input mesh file in off format
// -o Output mesh in off or vtk format
// -c center of the circle
// -r radius of the circle
// -e size of the exclusion zone around the line segment
// -p perturbation step size for node adjustment during clipping

#include <vm_MeshSlicer.h>
#include <vm_io.h>
#include <boost/bind.hpp>
#include <CLI/CLI.hpp>

double level_set_function(const std::vector<double>& center, const double radius, const double* X)
{
  return std::sqrt((X[0]-center[0])*(X[0]-center[0]) + (X[1]-center[1])*(X[1]-center[1]))-radius;
}


int main(int argc, char** argv)
{
  // Command line options
  std::string in_meshfile;       // input mesh file
  std::string out_meshfile;      // output mesh file
  std::vector<double> center{};  // center of the circle
  double radius;                 // radius of the circle
  double phi_tol;                // exclusion zone size around the segment
  double pert_tol;               // perturbation step size

  CLI::App app;
  app.add_option("-i", in_meshfile, "Input mesh file in off format")->required()->check(CLI::ExistingFile);
  app.add_option("-o", out_meshfile, "Output mesh in off or vtk format")->required()->check(!CLI::ExistingFile);
  app.add_option("-c", center, "coordinates of the center of the circle")->required()->expected(2);
  app.add_option("-r", radius, "radius of the circle")->required()->check(CLI::PositiveNumber);
  app.add_option("-e", phi_tol, "size of the exclusion zone around the line segment")->required()->check(CLI::PositiveNumber);
  app.add_option("-p", pert_tol, "perturbation step size for node adjustment during clipping")->required()->check(CLI::PositiveNumber);
  CLI11_PARSE(app, argc, argv);
  assert(std::filesystem::path(in_meshfile).extension()==".off" || std::filesystem::path(in_meshfile).extension()==".OFF");

  // Read the mesh
  pmp::SurfaceMesh mesh;
  vm::read_off(in_meshfile, mesh);

  // level set function
  vm::LevelSetFunction_t lsfunc = boost::bind(level_set_function, center, radius, _1);

  // adjust nodes away from the zero level set
  int nadjusted = vm::adjust_mesh_nodes(mesh, phi_tol, pert_tol, lsfunc);
  std::cout << "Adjusted " << nadjusted << " nodes near the circle" << std::endl;
  vm::write_off(mesh, std::string(std::filesystem::path(in_meshfile).stem())+"-adjusted.off");

  // clip
  vm::clip_mesh(mesh, 0.9*phi_tol, lsfunc);
  vm::write_off(mesh, out_meshfile);

  // done
}
  
