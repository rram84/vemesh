// Sriramajayam

#include <vm_visibility.h>
#include <vm_vertex_ring.h>
#include <vm_inspect.h>

// cgal visibility utilities
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Simple_polygon_visibility_2.h>
//#include <CGAL/Triangular_expansion_visibility_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arr_naive_point_location.h>
#include <CGAL/Boolean_set_operations_2.h>

#include <vm_io.h>

namespace vm
{
  // cgal aliases
  using Kernel                  = CGAL::Exact_predicates_exact_constructions_kernel;
  using Point_2                 = Kernel::Point_2;
  using Segment_2               = Kernel::Segment_2;
  using Polygon_2               = CGAL::Polygon_2<Kernel>;
  using Polygon_with_holes_2    = CGAL::Polygon_with_holes_2<Kernel>;
  using Traits_2                = CGAL::Arr_segment_traits_2<Kernel>;
  using Arrangement_2           = CGAL::Arrangement_2<Traits_2>;
  using Face_handle             = Arrangement_2::Face_handle;                                      
  using RSPV                    = CGAL::Simple_polygon_visibility_2<Arrangement_2, CGAL::Tag_false>; //CGAL::Triangular_expansion_visibility_2<Arrangement_2>; 


  // sanity checks intermediate visibility polygons
  template<class Polygon_t, class Point_t>
  void check_visibility_polygon(const Polygon_t& vp, const Point_t& pt)
  {
    // (i) should be counter-clockwise
    assert(vp.orientation()==CGAL::COUNTERCLOCKWISE);

    // (ii) should be simple
    assert(vp.is_simple()==true);

    // (iii) should have positive area
    assert(vp.area()>0.);

    // (iv) should contain X
    assert(vp.bounded_side(pt)==true);

    // done
    return;
  }
  
  
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
    //assert((*face)->is_unbounded()==false);
    if((*face)->is_unbounded()==true)
      {
	std::cout << "DETECTED UNBOUNDED FACE IN VISIBILITY CALCULATION. " << std::endl;
	std::cout << "Environment: " << std::endl;
	for(auto& v:vertex_ring)
	  {
	    const auto& Y = mesh.position(v);
	    std::cout << Y[0] << "  " << Y[1] << std::endl;
	  }
	std::cout << "Vertex: id = " << vertex.idx() << ", coord: " << X[0] << " " << X[1] << std::endl;
	vm::write_off(mesh, "problem-mesh.off");
	assert(false);
      }

    
    // compute the regularized visibility polygon from each of the guard vertices
    const double EPS = 0.01;
    RSPV regular_visibility(env);
    Arrangement_2 regular_output;
    std::vector<Polygon_2> visibility_polygons{};
    for(auto guard:vertex_guards)
      {
	regular_output.clear();
	const auto& Y = mesh.position(guard);
	const Point_2 guard_eps( (1.-EPS)*Y[0]+EPS*X[0], (1.-EPS)*Y[1]+EPS*X[1] );
	regular_visibility.compute_visibility(guard_eps, *face, regular_output);
	
	// this visibility polygon
	Polygon_2 vp;
	const int num_halfedges = regular_output.number_of_halfedges()/2;
	auto hedge = regular_output.halfedges_begin();
	for(int hcount=0; hcount<num_halfedges; ++hcount)
	  {
	    const auto& P = hedge->source()->point();
	    vp.push_back(Point_2(CGAL::to_double(P.x()), CGAL::to_double(P.y())));
	    hedge = hedge->next();
	  }

	// fix orientation
	if(vp.orientation()==CGAL::CLOCKWISE)
	  vp.reverse_orientation();
	
	// sanity checks
	check_visibility_polygon(vp, Point_2(X[0],X[1]));

	// append
	visibility_polygons.push_back(vp);
      }

    // intersect visibility polygons
    const int npolygons = static_cast<int>(visibility_polygons.size());
    Polygon_2 vis_poly = visibility_polygons[0];
    for(int i=0; i<npolygons; ++i)
      {
	// intersect vis_poly with visibility_polygons[i]
	std::vector<Polygon_with_holes_2> intersection{};
	CGAL::intersection(vis_poly, visibility_polygons[i], std::back_inserter(intersection));

	// expect one connected component with no holes
	assert(static_cast<int>(intersection.size())==1);
	assert(intersection[0].holes().empty()==true);
	
	// sanity checks on the intersection
	check_visibility_polygon(intersection[0].outer_boundary(), Point_2(X[0],X[1]));
	
	// update the visibility polygon to the intersection
	vis_poly.clear();
	vis_poly = intersection[0].outer_boundary();

	// next
      }

    // final check on the visibility polygon
    check_visibility_polygon(vis_poly, Point_2(X[0],X[1]));
    
    // return the vertices of the intersection polygon
    std::vector<std::pair<double,double>> poly_verts{};
    for (Polygon_2::Vertex_iterator vi=vis_poly.vertices_begin(); vi!=vis_poly.vertices_end(); ++vi)
      poly_verts.push_back({CGAL::to_double(vi->x()), CGAL::to_double(vi->y())});
    
    // inspect the correctness of a computed visibility polygon
    assert(inspect_visibility_polygon(mesh, vertex, poly_verts)==true);
    
    return std::move(poly_verts);
  }

}
