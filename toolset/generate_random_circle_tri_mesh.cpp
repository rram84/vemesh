// Sriramajayam

// generate a random delaunay triangulation over a unit circle

// Required options:
// -n: Number of random vertices to generate
// -m: Output mesh file name in off format, does not overwrite. Also writes a file in vtk format with element qualities

#include <vm_SpecialMeshes.h>
#include <vm_face_quality.h>
#include <vm_io.h>
#include <CLI/CLI.hpp>
#include <cmath>
#include <random>

// random points in a unit circle
std::vector<std::pair<double,double>> generate_points(const int num_points)
{
  // generate a random collection of points
  std::random_device rd;  
  std::mt19937 gen(rd()); 
  std::uniform_real_distribution<> rdis(0.,1.);
  std::uniform_real_distribution<> tdis(0.,2.*M_PI);
  std::vector<std::pair<double,double>> points(num_points);
  for(int i=0; i<num_points; ++i)
    {
      const double r = std::sqrt(rdis(gen));
      const double t = tdis(gen);
      points[i] = {r*std::cos(t), r*std::sin(t)};
    }
  return points;
}


int main(int argc, char** argv)
{
  // Either provide the number of sample points or the input mesh file
  CLI::App app;

  // Number of random points to generate over the unit circle
  int num_points;
  app.add_option("-n", num_points, "number of random points over unit circle")->required()->check(CLI::PositiveNumber);

  // mesh file
  std::string meshfile;
  app.add_option("-m", meshfile, "output mesh file with OFF extension")->required()->check(!CLI::ExistingFile);
  
  // parse
  CLI11_PARSE(app, argc, argv);
  const std::string ext = std::filesystem::path(meshfile).extension();
  assert((ext==".off" || ext==".OFF") && "Expected mesh file name in OFF format");
  
  // sampling
  auto points = generate_points(num_points);

  // mesh
  auto mesh = vm::create_delaunay_triangulation(points);
  vm::write_off(mesh, meshfile);

  // print mesh with quality in vtk format
  const std::string vtk_filename = std::string(std::filesystem::path(meshfile).stem())+".vtk";
  vm::write_vtk_with_cell_data(mesh, vm::compute_stiffness_based_mesh_face_quality, vtk_filename);

  // done
}
