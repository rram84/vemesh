// Sriramajayam

#include <vm_polygon_kernel.h>
#include <cassert>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <iostream>

namespace vm
{
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t    = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t  = bgm::polygon<boost_point_t, false>;

  
  std::vector<std::pair<double,double>>
  compute_polygon_kernel(const std::vector<pmp::Point>& vertices)
  {
    // bounding box 
    const auto& v0 = vertices.front();
    double xmin = v0[0];
    double xmax = v0[0];
    double ymin = v0[1];
    double ymax = v0[1];
    for(auto& pt:vertices)
      {
	const auto& x = pt[0];
	const auto& y = pt[1];
	if(x<xmin) xmin = x;
	if(x>xmax) xmax = x;
	if(y<ymin) ymin = y;
	if(y>ymax) ymax = y;
      }

    // diagonal of the bounding box
    const double dia = std::sqrt((xmax-xmin)*(xmax-xmin)+(ymax-ymin)*(ymax-ymin));

    // construct large quadrilaterals defining half-spaces for each edge
    std::vector<boost_polygon_t> half_spaces{};

    const int nVerts = static_cast<int>(vertices.size());
    for(int i=0; i<nVerts; ++i)
      {
	std::cout << " I am here " << std::endl;
	const auto& A = vertices[i];
	const auto& B = vertices[(i+1)%nVerts];

	// unit vector along AB 
	const double len    = std::sqrt((A[0]-B[0])*(A[0]-B[0]) + (A[1]-B[1])*(A[1]-B[1]));
	const double tvec[] = {(B[0]-A[0])/len, (B[1]-A[1])/len};

	// normal along AB
	const double nvec[] = {-tvec[1], tvec[0]};

	// this half-space
	const double P[] = {A[0]-dia*tvec[0], A[1]-dia*tvec[1]};
	const double Q[] = {B[0]+dia*tvec[0], B[1]+dia*tvec[1]};
	const double R[] = {Q[0]+dia*nvec[0], Q[1]+dia*nvec[1]};
	const double S[] = {P[0]+dia*nvec[0], P[1]+dia*nvec[1]};
	half_spaces.push_back(boost_polygon_t{{{P[0],P[1]}, {Q[0],Q[1]}, {R[0],R[1]}, {S[0],S[1]}, {P[0],P[1]}}});
      }

    // compute the kernel as the intersection of half-spaces
    boost_polygon_t kernel = half_spaces.back();
    half_spaces.pop_back();
    for(auto& quad:half_spaces)
      {
	std::vector<boost_polygon_t> intersections{};
	bg::intersection(kernel, quad, intersections);

	// kernel is assumed to be non empty, with one connected component
	assert(intersections.empty()==false);
	assert(static_cast<int>(intersections.size())==1);

	// update the kernel
	kernel = std::move(intersections[0]);
      }

    // vertices
    std::vector<std::pair<double,double>> kernel_verts{};
    for(auto& v:kernel.outer())
      kernel_verts.push_back({bg::get<0>(v), bg::get<1>(v)});

    // don't repeat the first <-> last vertex
    kernel_verts.pop_back();

    // done
    return std::move(kernel_verts);
  }
  
}
