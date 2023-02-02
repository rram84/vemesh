// Sriramajayam

#include <vm_polygon_sampling.h>
#include <boost/geometry/geometry.hpp>
#include <random>

namespace vm
{
  namespace bg  = boost::geometry;
  namespace bgm = boost::geometry::model;
  using boost_point_t      = bgm::point<double, 2, bg::cs::cartesian>;
  using boost_polygon_t    = bgm::polygon<boost_point_t>;
  using boost_box_t        = bgm::box<boost_point_t>;
  using boost_linestring_t = bgm::linestring<boost_point_t>;
  
  std::vector<std::pair<double,double>>
  compute_polygon_sampling(const std::vector<std::pair<double,double>>& vertices, const int num_points)
  {
    // this polygon
    const int nVerts = static_cast<int>(vertices.size());
    assert(nVerts>=2);
    boost_polygon_t    poly;
    boost_linestring_t ls;
    for(auto& v:vertices)
      {
	bg::append(poly.outer(), boost_point_t(v.first,v.second));
	bg::append(ls, boost_point_t(v.first,v.second));
      }

    // repeat the first vertex
    bg::append(poly.outer(), boost_point_t(vertices.front().first, vertices.front().second));
    bg::append(ls, boost_point_t(vertices.front().first, vertices.front().second));
    bg::correct(poly);
    bg::correct(ls);
    
    // average edge length- will be used for normalization
    const double avg_edge_len = bg::perimeter(poly)/static_cast<double>(nVerts);

    // tolerance to fence sample points away from the boundary
    const double BOUNDARY_FENCE_TOL = 0.025;
    
    // axis aligned bounding box
    boost_box_t envelope;
    boost::geometry::envelope(poly, envelope);

    // corners of the bounding box
    const auto& min_corner = envelope.min_corner();
    const auto& max_corner = envelope.max_corner();
    const double A[] = {bg::get<0>(min_corner), bg::get<1>(min_corner)};
    const double B[] = {bg::get<0>(max_corner), bg::get<1>(max_corner)};

    // uniform random distribution for sampling the bounding box
    std::random_device rd;  
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<> dis(0., 1.);

    // generate required number of samples within the polygon
    std::vector<std::pair<double,double>> samples{};
    int count = 0;
    while(count<num_points)
      {
	const double px = dis(gen);  // along the x-axis
	const double py = dis(gen);  // along the y-axis
	boost_point_t sample(px*A[0]+(1.-px)*B[0], py*A[1]+(1.-py)*B[1]);

	// does this sample lie in the polygon?
	if(bg::within(sample, poly)==true)
	  if(bg::distance(sample, ls)/avg_edge_len > BOUNDARY_FENCE_TOL)
	    {
	      samples.push_back({bg::get<0>(sample), bg::get<1>(sample)});
	      ++count;
	    }
      }
    
    // done
    return std::move(samples);
  }

}
