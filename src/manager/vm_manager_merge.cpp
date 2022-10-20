// Sriramajayam

#include <vm_manager.h>
#include <vm_quality.h>
#include <vm_merge.h>
#include <vm_inspect.h>
#include <set>
#include <map>

namespace vm
{
  // merge poor quality elements with neighbors
  void Manager::merge(const double eps_degrees)
  {
    // collate a list of faces with quality less than the threshold
    // sort them in ascending order of quality
    using Face_Quality_t = std::pair<pmp::Face, double>;
    auto cmp = [](const Face_Quality_t& A, const Face_Quality_t& B) { return A.second<B.second; };
    std::set<Face_Quality_t, decltype(cmp)> bad_faces(cmp);
    bad_faces.clear();
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	double quality = face_quality(mesh, face);
	if(quality <eps_degrees)
	  bad_faces.insert({face, quality});
      }
    
    // deal with bad faces sequentially
    while(!bad_faces.empty())
      {
	// pop the first face
	auto& face = bad_faces.begin()->first;

	// is this still a valid face
	if(mesh.is_valid(face) && !mesh.is_deleted(face))
	  merge_face(face);

	bad_faces.erase(bad_faces.begin());
      }

    return;
  }
  
  // merge a face with a neighbor in the mesh
  void Manager::merge_face(const pmp::Face& face)
  {
    // face needs to be merged with a neighbor
    // pick the neighbor so that the resulting face has the best quality among all possibilities

    // evaluate halfedge merged -> resulting face quality
    pmp::Halfedge best_h;
    double best_quality = -1.;
    auto halfedge_circulator = mesh.halfedges(face);
    for(auto h:halfedge_circulator)
      if(!mesh.is_boundary(h))
	{
	  auto nb_h    = mesh.opposite_halfedge(h);
	  auto nb_face = mesh.face(nb_h);
	  assert(mesh.is_valid(nb_face) && !mesh.is_deleted(nb_face));

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

    // there should be at least one candidate
    assert(best_quality>0. && mesh.is_valid(best_h));

    // merge along best_h
    vm::merge(mesh, best_h);

    // done
    return;
  }
  
}
