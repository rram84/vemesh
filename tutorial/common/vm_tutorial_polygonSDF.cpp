// Sriramajayam

/** \file vm_tutorial_polygonSDF.cpp
 * \brief Implementation of vm::tutorial::PolygonSDF
 * \author Ramsharan Rangarajan
 */

#include <vm_tutorial_polygonSDF.h>

#include <fstream>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <cstddef>

namespace vm
{
  namespace tutorial
  {
    namespace bgi = boost::geometry::index;

    // Construct from a vector of vertices
    PolygonSDF::PolygonSDF(const std::vector<double>& vertices)
    {
      build(vertices);
    }


    // Construct from vertices in a file
     PolygonSDF::PolygonSDF(const std::string& filename)
    {
      std::ifstream file(filename);
      if(!file.is_open())
        throw std::runtime_error("PolygonSDF: cannot open interface file " + filename);
      
      std::vector<double> vertices;
      double x, y;
      while(file >> x >> y)
        {
	  vertices.emplace_back(x);
	  vertices.emplace_back(y);
	}
      build(vertices);
    }

    // build the boost polygon and rtree for boundary segments
    void PolygonSDF::build(const std::vector<double>& vertices)
    {
      const int nvertices = static_cast<int>(vertices.size())/2;
      if(nvertices < 3)
        throw std::invalid_argument("PolygonSDF: polygon has fewer than 3 vertices");

      // close + orient the ring, then index its boundary segments
      vm::boost_polygon_t polygon;
      for(int n=0; n<nvertices; ++n)
        vm::bg::append(polygon.outer(), vm::boost_point_t(vertices[2*n], vertices[2*n+1]));
      vm::bg::correct(polygon);
      
      const auto& ring = polygon.outer();          // closed: ring.front()==ring.back()
      std::vector<segment_t> segs;
      segs.reserve(ring.size());
      for(std::size_t i = 0; i + 1 < ring.size(); ++i)
        segs.emplace_back(ring[i], ring[i + 1]);

      rtree_ = rtree_t(segs.begin(), segs.end());  // bulk-load (packing) constructor
    }
    

    // compute signed distance to the polygon --- //
    double PolygonSDF::operator()(double x, double y) const
    {
      const vm::boost_point_t P(x, y);

      // (1) exact distance: nearest boundary segment via the index
      std::vector<segment_t> nn;
      rtree_.query(bgi::nearest(P, 1), std::back_inserter(nn));
      const double dist = vm::bg::distance(P, nn.front());

      // (2) sign: +x horizontal ray-crossing count.
      // Only segments whose box meets the ray box can cross, so query just those.
      const double xmax = vm::bg::get<vm::bg::max_corner, 0>(rtree_.bounds()) + 1.0;
      const vm::boost_box_t ray_box(vm::boost_point_t(x, y),
                                    vm::boost_point_t(xmax, y));
      std::vector<segment_t> cand;
      rtree_.query(bgi::intersects(ray_box), std::back_inserter(cand));

      int crossings = 0;
      for(const auto& s : cand)
        {
          const double x0 = vm::bg::get<0, 0>(s), y0 = vm::bg::get<0, 1>(s);
          const double x1 = vm::bg::get<1, 0>(s), y1 = vm::bg::get<1, 1>(s);
          if( ((y0 > y) != (y1 > y)) &&
              (x < (x1 - x0) * (y - y0) / (y1 - y0) + x0) )
            ++crossings;
        }

      // odd number of boundary crossings  ->  point is inside  ->  negative distance
      const bool inside = (crossings % 2 == 1);
      return inside ? -dist : dist;
    }

  }
}
