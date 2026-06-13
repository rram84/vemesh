// Sriramajayam

#include <fstream>
#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/bind.hpp>

#include <vm_SpecialMeshes.h>
#include <vm_MeshSlicer.h>
#include <vm_io.h>

namespace bg = boost::geometry;
using boost_point2D      = bg::model::point<double, 2, boost::geometry::cs::cartesian>;
using boost_polygon2D    = bg::model::polygon<boost_point2D>;
using boost_linestring   = bg::model::linestring<boost_point2D>;

double level_set_func(const boost_polygon2D& poly, const boost_linestring& ls, const double* X);

int main()
{
  // read vertices
  std::fstream pfile;
  pfile.open("swan_vertices.dat", std::ios::in);

  // polygon
  boost_polygon2D poly;
  double xy[2];
  pfile >> xy[0];
  while(pfile.good())
    {
      pfile >> xy[1];
      bg::append(poly.outer(), boost_point2D(xy[0],xy[1]));
      pfile >> xy[0];
    }
  pfile.close();
  bg::correct(poly);
  
  // line string
  boost_linestring ls;
  for(auto& it:poly.outer())
    bg::append(ls, it);
  
  std::cout << "Polygon area: " << bg::area(poly) <<", perimeter: " << bg::perimeter(poly) << std::endl;
  bg::correct(ls);
  std::cout << "Linestring length: " << bg::length(ls) << std::endl;
  
  pfile.open("polverts.dat", std::ios::out);
  for(auto& it:poly.outer())
    pfile << bg::get<0>(it) << " " << bg::get<1>(it) << std::endl;
  pfile.close();

   pfile.open("lsrts.dat", std::ios::out);
   for(auto& it:ls)
     pfile << bg::get<0>(it) << " " << bg::get<1>(it) << std::endl;
   pfile.close();

   std::cout << poly.outer().size() << " and " << ls.size() << std::endl;
   
  // create rectangle mesh over a bounding box
  const double left_cnr[] = {-0.65,-0.455};
  const double h = 0.012345;
  const int nx = static_cast<int>(1.3/h);
  const int ny = static_cast<int>(0.9/h);
  auto rect_mesh = vm::create_rect_mesh(left_cnr, h, nx, h, ny);
  vm::write_vtk(rect_mesh, "bg.vtk");
  
  vm::LevelSetFunction_t lsfunc = boost::bind(level_set_func, poly, ls, _1);
  vm::clip_mesh(rect_mesh, 1.e-6, lsfunc);
  vm::write_vtk(rect_mesh, "clipped.vtk");
  vm::write_off(rect_mesh, "clipped.OFF");
}


double level_set_func(const boost_polygon2D& poly, const boost_linestring& ls, const double* X)
{
  bool is_inside = bg::within(boost_point2D(X[0],X[1]), poly);
  double dist = bg::distance(boost_point2D(X[0],X[1]), ls);
  if(is_inside==true)
    return -dist;
  else
    return dist;
}
