// Sriramajayam

/** \file vm_tutorial_polygonSDF.cpp
 * \brief Implementation of vm::tutorial::PolygonSDF and vm::tutorial::read_polygon_loops
 * \author Ramsharan Rangarajan
 */

#include <vm_tutorial_polygonSDF.h>

#include <fstream>
#include <stdexcept>
#include <vector>
#include <iterator>
#include <cstddef>
#include <string>
#include <sstream>

namespace vm
{
  namespace tutorial
  {
    namespace bgi = boost::geometry::index;

    // Construct from a vector of vertices
    PolygonSDF::PolygonSDF(const std::vector<double>& vertices)
    {
      build({vertices});
    }

    // Construct from several closed loops
    PolygonSDF::PolygonSDF(const std::vector<std::vector<double>>& loops)
    {
      build(loops);
    }
    
    // build one R-tree over the boundary segments of all loops
    void PolygonSDF::build(const std::vector<std::vector<double>>& loops)
    {
      std::vector<segment_t> segs;
      std::size_t npts = 0;
      for(const auto& v : loops) npts += v.size()/2;
      segs.reserve(npts + loops.size());        // ~ one closing segment per loop

      int nvalid = 0;
      for(const auto& vertices : loops)
        {
          const int nvertices = static_cast<int>(vertices.size())/2;
          if(nvertices < 3) continue;           // skip degenerate loop
          ++nvalid;

          vm::boost_polygon_t polygon;
          for(int n=0; n<nvertices; ++n)
            vm::bg::append(polygon.outer(),
                           vm::boost_point_t(vertices[2*n], vertices[2*n+1]));
          vm::bg::correct(polygon);             // close + orient this ring

          const auto& ring = polygon.outer();
          for(std::size_t i = 0; i + 1 < ring.size(); ++i)
            segs.emplace_back(ring[i], ring[i + 1]);
        }

      if(nvalid == 0)
        throw std::invalid_argument("PolygonSDF: no loop has at least 3 vertices");

      rtree_ = rtree_t(segs.begin(), segs.end());   // bulk-load once, over all loops
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


    // Read blank-line-separated loops from a text file
    std::vector<std::vector<double>> read_polygon_loops(const std::string& filename)
    {
      std::ifstream file(filename);
      if(!file.is_open())
        throw std::runtime_error("read_polygon_loops: cannot open file " + filename);

      std::vector<std::vector<double>> loops;
      std::vector<double> cur;
      std::string line;
      while(std::getline(file, line))
        {
          std::istringstream ss(line);
          double x, y;
          if(ss >> x >> y)
            { cur.emplace_back(x); cur.emplace_back(y); }
          else if(!cur.empty())              // blank/garbage line ends a loop
            { loops.emplace_back(std::move(cur)); cur.clear(); }
        }
      if(!cur.empty())
        loops.emplace_back(std::move(cur));  // final loop
      return loops;
    }
  }
}
