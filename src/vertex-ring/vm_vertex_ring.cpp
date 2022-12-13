// Sriramajayam

#include <vm_vertex_ring.h>
#include <set>

namespace vm
{
  // compute the vertex ring
  std::vector<pmp::Vertex> get_vertex_ring(const pmp::SurfaceMesh& mesh, const pmp::Vertex& v)
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

	// append vertices to the ring, exclude 'v' and repetitions
	for(int i=2; i<nVerts; ++i)
	  vertex_ring.push_back(face_vertices[i]);
      }

    // done
    return std::move(vertex_ring);
  }


  // inspect if a vertex is connected to a hanging node
  bool is_vertex_connected_to_hanging_node(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex)
  {
    // the one-ring of vertices should not have any repetitions
    const std::vector<pmp::Vertex> vertex_ring = get_vertex_ring(mesh, vertex);
    const int nRingVerts = static_cast<int>(vertex_ring.size());
    std::set<int> vertex_set{};
    for(auto& v:vertex_ring)
      vertex_set.insert(v.idx());

    // true if there is a repetition, false otherwise
    return (static_cast<int>(vertex_set.size())!=nRingVerts);
  }
  
}
