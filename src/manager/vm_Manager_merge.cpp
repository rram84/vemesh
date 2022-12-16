// Sriramajayam

#include <vm_Manager.h>
#include <vm_quality.h>
#include <vm_merge.h>
#include <vm_inspect.h>
#include <set>
#include <map>

namespace vm
{
  // merge poor quality elements with neighbors
  int Manager::merge_faces(const double eps_degrees)
  {
    int merge_count = 0;
    
    // collate a list of faces with quality less than the threshold
    // sort them in ascending order of quality
    using Face_Quality_t = std::pair<pmp::Face, double>;
    auto cmp = [](const Face_Quality_t& A, const Face_Quality_t& B) { return A.second<B.second; };
    std::set<Face_Quality_t, decltype(cmp)> bad_faces(cmp);
    bad_faces.clear();
    auto face_circulator = mesh.faces();
    for(auto face:face_circulator)
      {
	double quality = compute_angle_based_face_quality(mesh, face);
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
	  {
	    // merge along best possible neighbor
	    auto result = merge_halfedge(mesh, face);
	    const auto& success       = result.first;
	    const auto& best_halfedge = result.second;
	    if(success==true)
	      {
		vm::merge(mesh, best_halfedge);
		++merge_count;
	      }
	  }
	bad_faces.erase(bad_faces.begin());
      }

    return merge_count;
  }
  
  
}
