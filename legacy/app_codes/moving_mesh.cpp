// Sriramajayam

#include <vm_test_mesh_slicer.h>
#include <vm_test_rectangle_mesh.h>
#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_quality.h>
#include <CLI/CLI.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/bind.hpp>

#include <filesystem>
#include <cmath>
#include <cstdio>
#include <omp.h>

namespace bg = boost::geometry;
using boost_point2D      = bg::model::point<double, 2, boost::geometry::cs::cartesian>;
using boost_polygon2D    = bg::model::polygon<boost_point2D>;
using boost_linestring   = bg::model::linestring<boost_point2D>;


// signed distance function to a polygon
double polygon_signed_distance(const double* X, const boost_polygon2D& poly, const boost_linestring& ls) {
  bool is_inside = bg::within(boost_point2D(X[0],X[1]), poly);
  double dist = bg::distance(boost_point2D(X[0],X[1]), ls);
  if(is_inside==true)
    return -dist;
  else
    return dist;
}


int main() {
  const double left_cnr[] = {-2.25,-2.35};
  const int N = 149;
  pmp::SurfaceMesh bgmesh = vm::test::create_rect_mesh(left_cnr, 4.6/N, N+1, 4.6/N, N+1, 0);
  vm::write_suku_format(bgmesh, "regular");
  exit(1);
  
#ifdef _OPENMP
  std::cout << "OpenMP is available" << std::endl;
#endif

  // save statistics
  std::fstream statfile;
  statfile.open("stats-last.dat", std::ios::out);
  assert(statfile.good());
  
  // generate meshes
  //std::vector<int> indices{339};
  //std::cout<< indices.size() << std::endl;
  //for(int iiter=0; iiter<static_cast<int>(indices.size()); ++iiter) {
  //const int iter = indices[iiter];
  
#pragma omp parallel for schedule(dynamic, 1)
  for(int iter=136; iter<=139; ++iter) {
    
#pragma omp critical
    std::cout << "Geometry: " << iter << " on thread # " << omp_get_thread_num() << std::endl;
    
    // read the polygon vertices
    char filename[100];
    std::snprintf(filename, sizeof(filename), "vertices/catface_curve_%03d.dat", iter);
    std::fstream file;
    file.open(filename, std::ios::in);
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

    // this mesh
    pmp::SurfaceMesh pert_mesh = bgmesh;

    // adjust node positions away from the zero level sets
    const double sdtol = 1.e-4;
    bool is_adjusted = vm::test::adjust_mesh_nodes(pert_mesh, sdtol, 1.2*sdtol, sdfunc);
    if(is_adjusted==false)
      continue;
    
    bool is_mesh_ok = vm::inspect_mesh(pert_mesh);
    if(is_mesh_ok==false) {
      std::cout << "generated an invalid mesh" << std::endl;
      continue;
    }

    // embed
    vm::test::embed_interface(pert_mesh, sdtol, sdfunc, {1,2}, 1); // shifting tolerance, {mat1, mat2}, boundary node id
    std::snprintf(filename, sizeof(filename), "embed/embed_%03d.vtk", iter);
    vm::write_vtk(pert_mesh, std::string(filename));
    std::snprintf(filename, sizeof(filename), "embed/embed_%03d", iter);
    vm::write_suku_format(pert_mesh, std::string(filename));

    // agglomerate
    std::cout << "Agglomerating" << std::endl;
    
    // mesh manager for this mesh
    std::snprintf(filename, sizeof(filename), "embed/embed_%03d.vtk", iter);
    vm::Manager manager((std::string(filename)));
    const auto& embed_mesh = manager.get_mesh();
    
    // identify cut faces by accummulating faces around interface vertices
    auto interface_id = embed_mesh.get_vertex_property<int>("interface_id");
    auto allverts = embed_mesh.vertices();
      
    // agglomerate cut faces
    int nmerged_total = 0;
    int neigen_total = 0;
    int ncut_initial  = 0;
    
    for(int i=0; i<5; ++i)
      {
	std::set<pmp::Face> cutfaces{};
	for(auto v:allverts)
	  if(interface_id[v]==1) {
	    auto faces = embed_mesh.faces(v);
	    for(auto f:faces)
	      cutfaces.insert(f);
	  }
	if(i==0)
	  ncut_initial = static_cast<int>(cutfaces.size());
      
	int nmerged = manager.merge_faces(cutfaces, vm::FaceQuality::stiffness, 0.2, 1.2, nullptr);
	int neigen = manager.eigencount;
	nmerged_total += nmerged;
	neigen_total += neigen;
	std::cout << "Num merged = " << nmerged << std::endl;
	std::cout << "Eigencount = " << neigen << std::endl; //manager.eigencount << std::endl;
	if(nmerged==0)
	  break;
      }
      
    // save the agglomerated mesh
    std::snprintf(filename, sizeof(filename), "agg/agg_%03d.vtk", iter);
    vm::write_vtk(embed_mesh, std::string(filename));
    std::snprintf(filename, sizeof(filename), "agg/agg_%03d", iter);
    vm::write_suku_format(embed_mesh, std::string(filename));

    // save statistics
#pragma omp critical
    {
      statfile << iter << "\t" << ncut_initial << "\t" << nmerged_total << "\t" << neigen_total << std::endl;
      statfile.flush();
    }
  }
  statfile.close();
}
