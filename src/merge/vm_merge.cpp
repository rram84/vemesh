// Sriramajayam

#include <vm_merge.h>
#include <vm_inspect.h>
#include <vm_quality.h>
#include <list>

namespace vm
{
  // identify the face along which to merger a given face
  std::pair<bool, pmp::Halfedge> merge_halfedge(pmp::SurfaceMesh& mesh, const pmp::Face& face)
  {
    // face needs to be merged with a neighbor
    // pick the neighbor so that the resulting face has the best quality among all possibilities
    // cannot merge along boundary faces
    // cannot merge along faces that would result in an isolated vertex

    // evaluate halfedge merged -> resulting face quality
    pmp::Halfedge best_h;
    double best_quality = -1.;
    auto halfedge_circulator = mesh.halfedges(face);
    for(auto h:halfedge_circulator)
      if(mesh.valence(mesh.from_vertex(h))>2 && mesh.valence(mesh.to_vertex(h))>2)  // prevent isolated vertices
	{
	  auto nb_h    = mesh.opposite_halfedge(h);
	  auto nb_face = mesh.face(nb_h);
	  if(mesh.is_valid(nb_h) && mesh.is_valid(nb_face) && !mesh.is_deleted(nb_face))
	    {
	      // vertices of the new face created by merging face/nb_face
	      std::vector<pmp::Point> verts{};
	      
	      // vertices from the face of h
	      verts.push_back(mesh.position(mesh.to_vertex(h)));
	      auto it_h = mesh.next_halfedge(h);
	      while(it_h!=h)
		{
		  verts.push_back(mesh.position(mesh.to_vertex(it_h)));
		  it_h = mesh.next_halfedge(it_h);
		}
	  
	      // vertices from the face of nb_h
	      it_h = mesh.next_halfedge(nb_h);
	      while(it_h!=nb_h)
		{
		  verts.push_back(mesh.position(mesh.to_vertex(it_h)));
		  it_h = mesh.next_halfedge(it_h);
		}
	      verts.pop_back();

	      // quality of the candidate merged face
	      assert(inspect_face(verts)==true);
	      double quality = face_quality(verts);
	      if(quality>best_quality)
		{
		  best_quality = quality;
		  best_h       = h;
		}
	    }
	}

    // there should be at least one candidate
    if(best_quality>0. && mesh.is_valid(best_h))
      return {true, best_h};
    else
      return {false, best_h};
  }
  
  
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
