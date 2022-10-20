// Sriramajayam

#include <vm_merge.h>
#include <list>

namespace vm
{
  // Agglomerate poor quality elements
  void merge(pmp::SurfaceMesh& mesh, const pmp::Halfedge& h0)
  {
    // sanity checks 
    assert(mesh.is_valid(h0) && !mesh.is_deleted(h0) && !mesh.is_boundary(h0));

    // face of h0
    auto f0 = mesh.face(h0);
    assert(mesh.is_valid(f0) && !mesh.is_deleted(f0));

    // opposite edge
    auto h1 = mesh.opposite_halfedge(h0);
    assert(mesh.is_valid(h1) && !mesh.is_deleted(h1) && !mesh.is_boundary(h1));

    // opposite face
    auto f1 = mesh.face(h1);
    assert(mesh.is_valid(f1) && !mesh.is_deleted(f1));

    // merging f0 and f1 can create an isolated vertex
    // this can happen if either vertex of h0 has valency = 2
    // permit this to happen only along the boundary
    auto v0 = mesh.from_vertex(h0);
    auto v1 = mesh.to_vertex(h0);
    if(mesh.valence(v0)==2) assert(mesh.is_boundary(v0));
    if(mesh.valence(v1)==2) assert(mesh.is_boundary(v1));
    
    // vertices of the merged face
    std::vector<pmp::Vertex> new_face_vertices{};
	
    // vertices from face of h0
    new_face_vertices.push_back( mesh.to_vertex(h0) );
    auto h = mesh.next_halfedge(h0);
    while(h!=h0)
      {
	new_face_vertices.push_back( mesh.to_vertex(h) );
	h = mesh.next_halfedge(h);
      }
    auto last_vertex = new_face_vertices.front();
	
    // vertices from face of h1. note that vertices of h1 have already been added
    h = mesh.next_halfedge(h1);
    while(mesh.to_vertex(h)!=last_vertex)
      {
	new_face_vertices.push_back( mesh.to_vertex(h) );
	h = mesh.next_halfedge(h);
      }
    
    // check for the possibility of isolated vertices caused when deleting faces
    const int nvertices = mesh.n_vertices();
    const int nNewVerts = static_cast<int>(new_face_vertices.size());
    for(int n=0; n<nNewVerts; ++n)
      if(mesh.valence(new_face_vertices[n])==2 && mesh.is_boundary(new_face_vertices[n]))
	new_face_vertices[n] = mesh.add_vertex(mesh.position(new_face_vertices[n]));
    
    // delete faces of h0 and h1 (can also delete the edge of h0,h1)
    // notice that this does not delete the remaining halfedges of faces f0 and f1.
    // instead, the faces of the halfedges are invalidated
    mesh.delete_face(f0);
    mesh.delete_face(f1);
    assert( mesh.is_deleted(f0) && mesh.is_deleted(f1) &&
	    mesh.is_deleted(h0) && mesh.is_deleted(h1) );
    
    // create the new new face
    auto new_face = mesh.add_face(new_face_vertices);
    assert(mesh.is_valid(new_face));
    
    // #vertices should remain unchanged
    assert(mesh.n_vertices()==nvertices);
    
    // done
    return;
  }
  
}
