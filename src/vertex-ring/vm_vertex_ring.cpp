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


  // compute the average edge length emanating from a vertex
  double compute_average_edge_length_at_vertex(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex)
  {
    double hsum   = 0.;
    int    hcount = 0;
    
    auto h_circulator = mesh.halfedges(vertex);
    const auto& X     = mesh.position(vertex);
    for(auto h:h_circulator)
      {
	const auto& Y = mesh.position(mesh.to_vertex(h));
	hsum += std::sqrt((X[0]-Y[0])*(X[0]-Y[0])+(X[1]-Y[1])*(X[1]-Y[1]));
	++hcount;
      }
    return hsum/static_cast<double>(hcount);
  }
  
}
