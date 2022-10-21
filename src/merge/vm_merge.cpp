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

    const int nvertices = mesh.n_vertices();
    auto e = mesh.edge(h0);
    assert(mesh.is_removal_ok(e));
    mesh.remove_edge(e);

    // #vertices should remain unchanged
    assert(mesh.n_vertices()==nvertices);

    // done
    return;
  }
    
}
