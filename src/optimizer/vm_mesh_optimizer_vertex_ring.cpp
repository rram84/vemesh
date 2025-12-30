// Sriramajayam

#include <vm_mesh_optimizer.h>

namespace
{
  // Remove consecutive duplicate vertices that would create non-manifold edges.
  //
  // Assume that v represents a circular sequence of vertices.
  // If three consecutive vertices satisfy v[i-1] == v[i+1], the current and previous
  // vertices are removed to eliminate the non-manifold condition.
  void erase_nonmanifold_edges(std::vector<pmp::Vertex>& vec) {

    if (static_cast<int>(vec.size()) < 3) return; // No operation if there are fewer than 3 elements

    // lambdas for circular listing
    auto circular_previous = [&vec](auto it) { return it == vec.begin() ? vec.end() - 1 : it - 1; };
    auto circular_next = [&vec](auto it) { return it == vec.end() - 1 ? vec.begin() : it + 1;};

    for (auto it = vec.begin(); it != vec.end(); ) {
      auto prev = circular_previous(it);
      auto next = circular_next(it);

      if (*prev == *next) {
	it = vec.erase(it);
	it = vec.erase(circular_previous(it)); // Erase the previous element
	if (it == vec.end()) it = vec.begin(); // Adjust iterator for circularity
      } else {
	++it;
      }
    }
  }
}

namespace vm
{
  // compute the vertex ring
  std::vector<pmp::Vertex> MeshOptimizer::get_vertex_ring(const pmp::Vertex& v) const
  {
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
	for(int i=2; i<nVerts; ++i)
	  vertex_ring.push_back(face_vertices[i]);

	// remove non-manifold edges
	erase_nonmanifold_edges(vertex_ring);
      }
    
    // done
    return vertex_ring;
  }
  
}
