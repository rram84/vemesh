// Sriramajayam


#include <vm_test_mesh_slicer.h>
#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_quality.h>
#include <CLI/CLI.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/bind.hpp>

#include <random>
#include <filesystem>
#include <cmath>

#include <omp.h>

namespace bg = boost::geometry;
using boost_point2D      = bg::model::point<double, 2, boost::geometry::cs::cartesian>;
using boost_polygon2D    = bg::model::polygon<boost_point2D>;
using boost_linestring   = bg::model::linestring<boost_point2D>;

// Flags
// -c Clip the mesh and retain the negative level set
// -e Embed the zero level set as an interface in the mesh

// Options
// -i input mesh file in off or vtk file format
// -p cartesian coordinates of a point on the zero level set
// -a angle of the cut in degrees
// -t offset tolerance for signed distance values
// -o output mesh file in off or vtk file format

// signed distance function to a polygon
double polygon_signed_distance(const double* X, const boost_polygon2D& poly, const boost_linestring& ls) {
  bool is_inside = bg::within(boost_point2D(X[0],X[1]), poly);
  double dist = bg::distance(boost_point2D(X[0],X[1]), ls);
  if(is_inside==true)
    return -dist;
  else
    return dist;
}


// signed distance to a circle of radius 0.7
double circ_signed_distance(const double* X) {
  return std::sqrt(X[0]*X[0]+X[1]*X[1])-0.7;
}


int main(int argc, char** argv) {

  // Command line options
  CLI::App app;
  app.footer("Embed a polygonal geometry within a triangle mesh. \n \
              Sample usage: ./embed_polygon_mesh -g polyvertices.dat -o outdir");

  // options
  std::string geom_file, outdir;
  double angle, sdtol;
  app.add_option("-g", geom_file, "input polygon vertices")->required()->check(CLI::ExistingFile);
  app.add_option("-o", outdir, "output directory")->required();

  // parse
  CLI11_PARSE(app, argc, argv);

  // read the polygon vertices
  std::fstream file;
  file.open(geom_file, std::ios::in);
  assert(file.good());
  boost_polygon2D poly;
  double xy[2];
  file >> xy[0];
  while(file.good()) {
    file >> xy[1];
    bg::append(poly.outer(), boost_point2D(1.5*xy[0],1.5*xy[1]));
    file >> xy[0];
  }
  file.close();
  bg::correct(poly);

  // line string
  boost_linestring ls;
  for(auto& it:poly.outer())
    bg::append(ls, it);

  // signed distance 
  vm::test::LevelSetFunction_t sdfunc = std::bind(polygon_signed_distance, std::placeholders::_1, poly, ls);
  //vm::test::LevelSetFunction_t sdfunc = circ_signed_distance;
  
  // read the input mesh
  const std::string in_meshfile = "bbbb-3.off";
  const double hval = 0.042;
  pmp::SurfaceMesh mesh;
  vm::read_off(in_meshfile, mesh);
  
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

  for(int iter=920; iter<950; ++iter) {

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
    
    // embed
    vm::test::embed_interface(pert_mesh, sdtol, sdfunc, {1,2}, 1); // shifting tolerance, {mat1, mat2}, boundary node id
    
    // save
    vm::write_vtk(pert_mesh, outdir+"/embed-"+std::to_string(iter)+".vtk");
    vm::write_suku_format(pert_mesh, outdir+"/embed-"+std::to_string(iter));
    
    // Mesh manager
    vm::Manager manager(outdir+"/embed-"+std::to_string(iter)+".vtk");
    manager.merge_faces(vm::FaceQuality::stiffness, 0.2, 1.2, nullptr);
    vm::write_vtk(manager.get_mesh(), outdir+"/embed-a-"+std::to_string(iter)+".vtk");
    vm::write_suku_format(manager.get_mesh(), outdir+"/embed-a-"+std::to_string(iter));
  }
  
  // done
}
