// Sriramajayam

#include <vm_vertex_sampling.h>
#include <boost/geometry/geometry.hpp>
#include <random>

namespace vm
{
  namespace bg             = boost::geometry;
  namespace bgm            = boost::geometry::model;
  using boost_point_t      = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t    = bgm::polygon<boost_point_t>;
  using boost_box_t        = bgm::box<boost_point_t>;
  using boost_linestring_t = bgm::linestring<boost_point_t>;

  // Query whether a point is feasible
  bool is_point_feasible(const boost_polygon_t         &poly,
			 const std::vector<pmp::Point> &connectedVertices,
			 const boost_point_t           &sample)
  {
    // does this point lie within the polygon
    if(bg::within(sample, poly)==false)
      return false;
    
    // do the segments joining this point to the connected vertices lie within the polygon?
    const double EPS = 0.01;
    for(auto& Y:connectedVertices)
      {
	boost_linestring_t seg;
	bg::append(seg, sample);
	bg::append(seg, boost_point_t(EPS*bg::get<0>(sample)+(1.-EPS)*Y[0],
				      EPS*bg::get<1>(sample)+(1.-EPS)*Y[1])); // boundary vertex moved inward
	if(bg::within(seg, poly)==false)
	  return false;
      }

    // done
    return true;
  }
  
    
  
  // random generation of feasible vertex positions
  std::vector<std::pair<double,double>>
  compute_feasible_vertex_positions(const pmp::SurfaceMesh &mesh,
				    const pmp::Vertex      &vertex,
				    const int              num_poly_samples,        // max number of random positions to generate
				    const int              num_edge_samples)        // number of samples to generate per edge  
  {
    // output
    std::vector<std::pair<double,double>> feasible_points{};
    
    // boost polygon of the environment around the vertex
    boost_polygon_t poly;
    auto v_circulator = mesh.vertices(vertex);
    for(auto v:v_circulator)
      {
	const auto& X = mesh.position(v);
	bg::append(poly.outer(), boost_point_t(X[0], X[1]));
      }
    bg::correct(poly);

    // axis-aligned bounding box for poly
    boost_box_t bbox;
    bg::envelope(poly, bbox);
    const auto& min_corner = bbox.min_corner();
    const auto& max_corner = bbox.max_corner();

    // outgoing halfedges from vertex
    auto out_halfedges = mesh.halfedges(vertex);

    // neighbors to which "vertex" is connected
    std::vector<pmp::Point> connected_vertices{};
    for(auto h:out_halfedges)
      {
	assert(mesh.from_vertex(h)==vertex);
	connected_vertices.push_back(mesh.position(mesh.to_vertex(h)));
      }
        
    // Random generator
    std::random_device rd; 
    std::mt19937 gen(rd());

    // uniform sampling over the x edge of the bounding box
    std::uniform_real_distribution<> xdis(bg::get<0>(min_corner), bg::get<0>(max_corner));
    std::uniform_real_distribution<> ydis(bg::get<1>(min_corner), bg::get<1>(max_corner));
    
    // sample the bounding box
    for(int iter=0; iter<num_poly_samples; ++iter)
      {
	// sample point in the bounding box
	boost_point_t sample(xdis(gen), ydis(gen));

	// is this point feasible
	if(is_point_feasible(poly, connected_vertices, sample)==true)
	  feasible_points.push_back({bg::get<0>(sample), bg::get<1>(sample)});
      }


    // sample edges incident edges at the vertex
    std::uniform_real_distribution<> lambda_dis(0.,1.);
    const pmp::Point& Xv = mesh.position(vertex);
    for(auto& Y:connected_vertices)
      for(int iter=0; iter<num_edge_samples; ++iter)
	{
	  const double lambda = lambda_dis(gen);
	  boost_point_t sample(lambda*Xv[0]+(1.-lambda)*Y[0],
			       lambda*Xv[1]+(1.-lambda)*Y[1]);
	  
	  if(is_point_feasible(poly, connected_vertices, sample)==true)
	    feasible_points.push_back({bg::get<0>(sample), bg::get<1>(sample)});
	}

    // done
    return std::move(feasible_points);
  }
  
} // vm::
