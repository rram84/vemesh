// Sriramajayam

// generate a random delaunay triangulation over [0,1]^2

// Required options:
// -n: Number of interior random vertices to generate
// -b: Number of uniformly spaced boundary vertices, assumed to be a multiple of 4
// -m: Output mesh file name in off format, does not overwrite. Also writes a file in vtk format with element qualities

#include <vm_SpecialMeshes.h>
#include <vm_face_quality.h>
#include <vm_io.h>
#include <CLI/CLI.hpp>
#include <random>

// random points in a unit square
std::vector<std::pair<double,double>> generate_points(const int num_int_points, const int num_bd_points)
  {
    // generate a random collection of interior points 
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<> dis(0.,1.);
    std::vector<std::pair<double,double>> points(num_int_points);
    for(int i=0; i<num_int_points; ++i)
      points[i] = {dis(gen), dis(gen)};
        
    // uniformly spaced boundary points
    const int nbd_per_side = num_bd_points/4;
    const double ds = 1./static_cast<double>(nbd_per_side);
    for(int i=0; i<nbd_per_side; ++i)
      {
	points.push_back({i*ds, 0.});
	points.push_back({1., i*ds});
	points.push_back({1.-i*ds, 1.});
	points.push_back({0., 1.-i*ds});
      }

    return points;
  }


int main(int argc, char** argv)
{
  // Either provide the number of sample points or the input mesh file
  CLI::App app;

  // Number of random points to generate over the region [0,1] x [0,1]
  int num_int_points;
  app.add_option("-n", num_int_points, "number of random points over unit square")->required()->check(CLI::PositiveNumber);

  // Number of uniformly spaced boundary points
  int num_bd_points;
  app.add_option("-b", num_bd_points, "number of uniformly spaced boundary points, multiple of 4")->required()->check(CLI::PositiveNumber);
  
  // mesh file
  std::string meshfile;
  app.add_option("-m", meshfile, "output mesh file with OFF extension")->required()->check(!CLI::ExistingFile);
  
  // parse
  CLI11_PARSE(app, argc, argv);
  const std::string ext = std::filesystem::path(meshfile).extension();
  assert((ext==".off" || ext==".OFF") && "Expected mesh file name in OFF format");
  assert(num_bd_points%4==0 && "number of boundary points should be a multiple of 4");

  // generate vertices
  auto points = generate_points(num_int_points, num_bd_points);
  
  // delaunay mesh
  auto mesh = vm::create_delaunay_triangulation(points);
  vm::write_off(mesh, meshfile);

  // print mesh with quality in vtk format
  const std::string vtk_filename = std::string(std::filesystem::path(meshfile).stem())+".vtk";
  vm::write_vtk_with_cell_data(mesh, vm::compute_stiffness_based_mesh_face_quality, vtk_filename);

  // done
}
