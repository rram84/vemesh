// Sriramajayam

#include <vm_Manager.h>
#include <vm_quality.h>
#include <vm_merge.h>
#include <vm_inspect.h>
#include <set>
#include <map>


namespace vm
{
  // merge poor quality triangles with neighbors
  int Manager::agglomerate_triangles(const double eps_degrees)
  {
    int agg_count = 0;

    // collate a list of triangles with quality less than the threshold
    // sort them in ascending order of quality
    using Face_Quality_t = std::pair<pmp::Face, double>;
    auto cmp = [](const Face_Quality_t& A, const Face_Quality_t& B) { return A.second<B.second; };
    std::set<Face_Quality_t, decltype(cmp)> bad_trias(cmp);
    bad_trias.clear();
    auto f_circulator = mesh.faces();
    for(auto face:f_circulator)
      if(mesh.valence(face)==3)
	{
	  double quality = compute_angle_based_face_quality(mesh, face);
	  if(quality<eps_degrees)
	    bad_trias.insert({face,quality});
	}

    // deal with bad trias sequentially
    while(!bad_trias.empty())
      {
	// pop the first face
	auto& tria = bad_trias.begin()->first;

	// is this still a valid triangle
	if(mesh.is_valid(tria) && !mesh.is_deleted(tria) && mesh.valence(tria)==3)
	  {
	    // merge along the best possible neighbor
	    auto result      = merge_halfedge(mesh, tria);
	    auto& success    = result.first;
	    auto& best_hedge = result.second;
	    if(success==true)
	      {
		vm::merge(mesh, best_hedge);
		++agg_count;
	      }
	  }
	bad_trias.erase(bad_trias.begin());
      }

    // inspect the mesh
    inspect_mesh();
    
    // done
    return agg_count;
  }
  
}
