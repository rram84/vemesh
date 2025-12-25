// Sriramajayam

#include <vm_utils.h>

namespace vm
{
  // create a boost polygon from a set of vertices
  boost_polygon_t make_polygon(const std::vector<pmp::Point>& coords)
  {
    assert(static_cast<int>(coords.size())>=3);
    
    // boost polygon representation
    boost_polygon_t poly;
    for(auto& pt:coords)
      bg::append(poly.outer(), boost_point_t(pt[0],pt[1]));

    auto first_vertex = *poly.outer().begin();
    bg::append(poly.outer(), first_vertex);
    return poly;
  }
}
