// Sriramajayam

#include <vm_visibility.h>
#include <vm_vertex_ring.h>

// cgal visibility utilities
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Simple_polygon_visibility_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>

// boost polygon utilities
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <vm_io.h>

namespace vm
{
  // cgal aliases
  using Kernel                  = CGAL::Exact_predicates_exact_constructions_kernel;
  using Point_2                 = Kernel::Point_2;
  using Segment_2               = Kernel::Segment_2;
  using Traits_2                = CGAL::Arr_segment_traits_2<Kernel>;
  using Arrangement_2           = CGAL::Arrangement_2<Traits_2>;
  using Face_handle             = Arrangement_2::Face_handle;                                      
  using RSPV                    = CGAL::Simple_polygon_visibility_2<Arrangement_2, CGAL::Tag_false>;
  
  // boost aliases
  namespace bg  = boost::geometry;
  namespace bgm = bg::model;
  using boost_point_t    = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t  = bgm::polygon<boost_point_t, false>;
  
  std::vector<std::pair<double,double>>
  compute_visibility_polygon(const pmp::SurfaceMesh& mesh,
			    const pmp::Vertex& vertex)
  {
    // vertex ring
    const auto vertex_ring = get_vertex_ring(mesh, vertex);

    // connected vertices
    auto vertex_guards = mesh.vertices(vertex);

    // create the environment in CGAL
    const int nRingVerts = static_cast<int>(vertex_ring.size());
    std::vector<Point_2> env_vertices{};
    for(int n=0; n<nRingVerts; ++n)
      {
	const auto& A = mesh.position(vertex_ring[n]);
	env_vertices.push_back(Point_2(A[0],A[1]));
      }
    std::vector<Segment_2> segments{}; 
    for(int n=0; n<nRingVerts; ++n)
      segments.push_back( Segment_2(env_vertices[n], env_vertices[(n+1)%nRingVerts]) );
    
    Arrangement_2 env;
    CGAL::insert_non_intersecting_curves(env, segments.begin(), segments.end());
    
    // cgal face of the evironment
    const auto& X = mesh.position(vertex);
    Arrangement_2::Face_const_handle *face;
    CGAL::Arr_naive_point_location<Arrangement_2> pl(env);
    CGAL::Arr_point_location_result<Arrangement_2>::Type obj = pl.locate(Point_2(X[0],X[1]));
    face = boost::get<Arrangement_2::Face_const_handle> (&obj);

    // sanity check
    assert((*face)->is_unbounded()==false);
    
    // compute the regularized visibility polygon from each of the guard vertices
    const double EPS = 0.01;
    RSPV regular_visibility(env);
    Arrangement_2 regular_output;
    std::vector<boost_polygon_t> visibility_polygons{};
    for(auto guard:vertex_guards)
      {
	regular_output.clear();
	const auto& Y = mesh.position(guard);
	const Point_2 guard_eps( (1.-EPS)*Y[0]+EPS*X[0], (1.-EPS)*Y[1]+EPS*X[1] );
	regular_visibility.compute_visibility(guard_eps, *face, regular_output);
	
	// this visibility polygon
	boost_polygon_t vp;
	const int num_halfedges = regular_output.number_of_halfedges()/2;
	auto hedge = regular_output.halfedges_begin();
	for(int hcount=0; hcount<num_halfedges; ++hcount)
	  {
	    const auto& P = hedge->source()->point();
	    bg::append(vp.outer(), boost_point_t(CGAL::to_double(P.x()), CGAL::to_double(P.y())));
	    hedge = hedge->next();
	  }
	// repeat the first vertex
	{
	  const auto& P = hedge->source()->point();
	  bg::append(vp.outer(), boost_point_t(CGAL::to_double(P.x()), CGAL::to_double(P.y())));
	}
	bg::correct(vp);

	// sanity checks
	bg::is_valid(vp);
	bg::is_simple(vp);

	// append
	visibility_polygons.push_back(vp);
      }
    
    
    // intersection of visibility polygones
    boost_polygon_t poly = visibility_polygons.back();
    visibility_polygons.pop_back();
    for(auto& vp:visibility_polygons)
      {
	std::vector<boost_polygon_t> intersections{};
	bg::intersection(poly, vp, intersections);

	// intersection should be non empty, with one connected component
	assert(intersections.empty()==false);
	assert(static_cast<int>(intersections.size())==1);

	// update the kernel
	poly = std::move(intersections[0]);

	// sanity checks
	bg::is_valid(poly);
	bg::is_simple(poly);
	assert(bg::within(boost_point_t(X[0],X[1]), poly)==true);
      }

    // return the vertices of the intersection polygon
    std::vector<std::pair<double,double>> poly_verts{};
    for(auto& v:poly.outer())
      poly_verts.push_back({bg::get<0>(v), bg::get<1>(v)});
    poly_verts.pop_back();

    return std::move(poly_verts);
  }

}
