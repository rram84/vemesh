// Sriramajayam

#include <vm_move.h>
#include <iostream>
#include <cassert>
#include <fstream>
#include <vm_io.h>
#include <vm_visibility.h>
#include <vm_quality.h>

#include <boost/geometry/geometry.hpp>

namespace bg  = boost::geometry;
namespace bgm = boost::geometry::model;
using boost_point_t   = bgm::point<double, 2, bg::cs::cartesian>;
using boost_polygon_t = bgm::polygon<boost_point_t>;
using boost_box_t     = bgm::box<boost_point_t>;


int main()
{
  // environment
  const double coords[][2] = {{0.9435, 0.2205},
			      {0.949581, 0.207981},
			      {0.8916, 0.1322},
			      {0.9405, 0.0363},
			      {0.9862, 0.6461},
			      {0.9529, 0.6696},
			      {0.9048, 0.6964},
			      {0.6569, 0.5915},
			      {0.7876, 0.4297},
			      {0.85372, 0.319876}};

    
  const int nVertices = 10;
   
  // guard
  const double guard[] = {0.872102, 0.42904}; 
  //const double guard[] = {0.9197, 0.4968}; 
  
  std::fstream pfile;
  pfile.open("env.dat", std::ios::out);
  for(int n=0; n<nVertices; ++n)
    pfile << coords[n][0] << " " << coords[n][1] << std::endl;
  pfile << coords[0][0] << " " << coords[0][1] << std::endl;
  pfile << std::endl << guard[0] << " " << guard[1] << std::endl; 
  pfile.close();

  std::vector<pmp::Vertex> vertices{};
  pmp::SurfaceMesh mesh;
  for(int n=0; n<nVertices; ++n)
    vertices.push_back(mesh.add_vertex(pmp::Point(coords[n][0],coords[n][1],0.)));

  // guard location
  auto guard_vertex = mesh.add_vertex(pmp::Point(guard[0],guard[1],0.));

  // add faces
  mesh.add_face({guard_vertex, vertices[5], vertices[6], vertices[7]});
  mesh.add_face({guard_vertex, vertices[7], vertices[8], vertices[9]});
  mesh.add_face({guard_vertex, vertices[9], vertices[0], vertices[1]});
  mesh.add_face({guard_vertex, vertices[1], vertices[2], vertices[3], vertices[4], vertices[5]});
  
  vm::write_off(mesh, "mesh.OFF");

  // halfedges at the vertex
  {
    pfile.open("in-edges.dat", std::ios::out);
    auto h_circulator = mesh.halfedges(guard_vertex);
    for(auto h:h_circulator)
      {
	const auto& X = mesh.position(mesh.from_vertex(h));
	const auto& Y = mesh.position(mesh.to_vertex(h));
	pfile << X[0] << " " << X[1] << std::endl
	      << Y[0] << " " << Y[1] << std::endl << std::endl;
      }
    pfile.close();
  }

  // compute the minimum distance of ring vertices to the edges incident at a vertex
  //double  min_dist = vm::compute_minimum_ring_vertices_to_inner_halfedges_distance(mesh, guard_vertex);
  //std::cout << "Min distance: " << min_dist << std::endl;

  // compute the visibility polygon
  auto vis_poly = vm::compute_visibility_polygon(mesh, guard_vertex);
  {
    pfile.open("vis.dat", std::ios::out);
    for(auto& it:vis_poly)
      pfile << it.first << " " << it.second << std::endl;
    pfile << vis_poly.front().first << " " << vis_poly.front().second << std::endl;
    pfile.close();
  }

  exit(1);
  
  // boost visibility polygon
  const int nVerts = static_cast<int>(vis_poly.size());
  assert(nVerts>=2);
  boost_polygon_t poly;
  for(auto& v:vis_poly)
    bg::append(poly.outer(), boost_point_t(v.first,v.second));

  // repeat the first vertex
  bg::append(poly.outer(), boost_point_t(vis_poly.front().first, vis_poly.front().second));
  bg::correct(poly);

  boost_point_t sample(0.229988, 0.937265);
  
  // does this sample lie in the polygon?
  if(bg::within(sample, poly)==true)
    {
      std::cout <<  std::endl << "PROBLEM DETECTED " << std::endl;
    }
  
}
