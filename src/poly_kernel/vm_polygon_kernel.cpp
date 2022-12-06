// Sriramajayam

#include <vm_polygon_kernel.h>
#include <cassert>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace vm
{
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t    = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t  = bgm::polygon<boost_point_t, false>;

  
  std::vector<std::pair<double,double>>
  compute_polygon_kernel(const pmp::SurfaceMesh& mesh, const pmp::Face& face)
  {
    assert(mesh.is_valid(face)==true);
    
    // bounding box for this face
    auto vertex_circulator = mesh.vertices(face);
    double xmin, xmax, ymin, ymax;
    // initialize
    for(auto v:vertex_circulator)
      {
	const auto& pt = mesh.position(v);
	xmin = pt[0];
	xmax = pt[0];
	ymin = pt[1];
	ymax = pt[1];
	break;
      }
    // bounds
    for(auto v:vertex_circulator)
      {
	const auto& pt = mesh.position(v);
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
    
    auto halfedge_circulator = mesh.halfedges(face);
    for(auto h:halfedge_circulator)
      {
	const auto& A = mesh.position(mesh.from_vertex(h));
	const auto& B = mesh.position(mesh.to_vertex(h));

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
