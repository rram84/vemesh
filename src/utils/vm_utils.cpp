// Sriramajayam

/** \file vm_utils.cpp
 * \brief Implements Boost.Geometry and pmp::SurfaceMesh related utility functions
 * \author Ramsharan Rangarajan
 */

#include <vm_utils.h>

// ---- Create a boost polygon from a set of points --- //
namespace vm
{
  // create a boost polygon from a set of vertices
  boost_polygon_t make_polygon(const std::vector<pmp::Point>& coords)
  {
    if(static_cast<int>(coords.size())<3)
      throw std::runtime_error("make_polygon: polygon should have at least 3 vertices");
    
    // boost polygon representation
    boost_polygon_t poly;
    for(auto& pt:coords)
      bg::append(poly.outer(), boost_point_t(pt[0],pt[1]));

    auto first_vertex = *poly.outer().begin();
    bg::append(poly.outer(), first_vertex);
    return poly;
  }
}

// ---- Compute the ring around an interior vertex in a mesh --- //
namespace
{
  // Remove consecutive duplicate vertices that would create non-manifold edges.
  //
  // Assume that v represents a circular sequence of vertices.
  // If three consecutive vertices satisfy v[i-1] == v[i+1], the current and previous
  // vertices are removed to eliminate the non-manifold condition.
  void erase_nonmanifold_edges(std::vector<pmp::Vertex>& vec) {
    std::size_t prev_size;
    do {
      prev_size = vec.size();
      if (vec.size() < 3) return;   // re-checked on every restart

      auto circular_previous = [&vec](auto it) { return it == vec.begin() ? vec.end() - 1 : it - 1; };
      auto circular_next     = [&vec](auto it) { return it == vec.end() - 1 ? vec.begin() : it + 1; };

      for (auto it = vec.begin(); it != vec.end(); ) {
	auto prev = circular_previous(it);
	auto next = circular_next(it);
	if (*prev == *next) {
	  it = vec.erase(it);
	  it = vec.erase(circular_previous(it));
	  if (it == vec.end()) it = vec.begin();
	} else {
	  ++it;
	}
      }
    } while (vec.size() != prev_size && vec.size() >= 3);
  }
}

   

namespace vm
{
  // compute the vertex ring
  std::vector<pmp::Vertex> get_environment_vertices(const pmp::Vertex& v, const pmp::SurfaceMesh &mesh)
  {
    if(mesh.is_boundary(v))
      throw std::runtime_error("get_environment_vertices: Cannot be used with a boundary vertex");
    
    // list to return
    std::vector<pmp::Vertex> vertex_ring{};
    
    // faces incident at v
    auto face_circulator = mesh.faces(v);

    for(auto f:face_circulator)
      {
	// vertices of this face
	auto vert_circulator = mesh.vertices(f);
	std::vector<pmp::Vertex> face_vertices{};
	int v_indx = -1;
	int indx = 0;
	for(auto w:vert_circulator)
	  {
	    face_vertices.push_back(w);
	    if(w.idx()==v.idx())
	      v_indx = indx;
	    ++indx;
	  }
	const int nVerts = static_cast<int>(face_vertices.size());
	
	// permute the list of face vertices until 'v' appears first
	std::rotate(face_vertices.begin(), face_vertices.begin()+v_indx, face_vertices.end());
	assert(face_vertices.front().idx()==v.idx());

	// append vertices to the ring, exclude 'v'
	// start from 2 to avoid successive repitition
	for(int i=2; i<nVerts; ++i)
	  vertex_ring.push_back(face_vertices[i]);

	// if starting from i=1, use this:
	// Remove linear consecutive duplicates
	//auto it = std::unique(vec.begin(), vec.end());
	//vec.erase(it, vec.end());
	
	// Fix circular adjacency (first == last)
	//if (vec.size() > 1 && vec.front() == vec.back())
	//vec.pop_back();
      }
    
    // remove non-manifold edges
    erase_nonmanifold_edges(vertex_ring);
    
    // done
    return vertex_ring;
  }
}


// ---- Create a boost polygon for the environment around a vertex in a mesh --- //
namespace vm
{
  boost_polygon_t get_environment_polygon(const pmp::Vertex& v, const pmp::SurfaceMesh& mesh)
  {
    // boost polygon of the environment around the vertex
    boost_polygon_t poly;
    auto vertex_ring = get_environment_vertices(v, mesh);
    for(auto& v:vertex_ring)
      {
	const auto& X = mesh.position(v);
	bg::append(poly.outer(), boost_point_t(X[0], X[1]));
      }
    bg::correct(poly);
    return poly;
  }
}

// ---- Get the list of vertices connected to a vertex in a mesh --- //
namespace vm
{
  std::vector<pmp::Point> get_connected_vertices(const pmp::Vertex &v, const pmp::SurfaceMesh& mesh)
  {
    // outgoing halfedges from vertex
    auto out_halfedges = mesh.halfedges(v);
  
    // neighbors to which "vertex" is connected
    std::vector<pmp::Point> connected_vertices{};
    for(auto h:out_halfedges)
      connected_vertices.push_back(mesh.position(mesh.to_vertex(h)));

    return connected_vertices;
  }
}
