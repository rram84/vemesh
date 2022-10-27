// Sriramajayam

#include <vm_snap.h>
#include <vm_inspect.h>
#include <set>
#include <map>
#include <list>

namespace vm
{
  // examine whether snapping a src_vertex to tgt_vertex in a mesh is ok
  bool is_snap_ok(const pmp::SurfaceMesh& mesh, const pmp::Vertex& src_vertex, const pmp::Vertex& tgt_vertex)
  {
    auto face_circulator = mesh.faces(src_vertex);
    for(auto face:face_circulator)
      {
	// vertices of this face
	auto vert_circulator = mesh.vertices(face);
	std::vector<pmp::Vertex> vertices{};
	for(auto vert:vert_circulator)
	  vertices.push_back(vert);
	
	// replace all occurrences of "src_vertex" by "tgt_vertex"
	for(auto& vert:vertices)
	  if(vert==src_vertex)
	    vert = tgt_vertex;

	// inspect the quality of each "cycle" in the resulting face
	int count = std::count(vertices.begin(), vertices.end(), tgt_vertex);
	if(count==1) // 1 cycle only
	  {
	    std::vector<pmp::Point> vert_coords{};
	    for(auto vert:vertices)
	      vert_coords.push_back(mesh.position(vert));
	    if(inspect_face(vert_coords)==false)
	      return false;
	  }
	else // the new face has multiple cycles
	  {
	    // permute until "tgt_vertex" appears at the start of the list
	    while(vertices.front()!=tgt_vertex)
	      std::rotate(vertices.begin(), vertices.begin()+1, vertices.end());
	    
	    // examine cycles from "tgt_vertex" to "tgt_vertex"
	    std::list<pmp::Vertex> vert_list(vertices.begin(), vertices.end());
	    std::vector<pmp::Point> vert_coords{};
	    vert_coords.push_back(mesh.position(vert_list.front()));
	    vert_list.pop_front();		
	    while(!vert_list.empty())
	      {
		if(vert_list.front()!=tgt_vertex)
		  vert_coords.push_back(mesh.position(vert_list.front()));
		else
		  {
		    // inspect the quality of this cycle
		    if(static_cast<int>(vert_coords.size())>2)
		      if(inspect_face(vert_coords)==false)
			return false;
		     
		    // prepare for the next cycle
		    vert_coords.clear();
		    vert_coords.push_back(mesh.position(vert_list.front()));
		  }
		
		vert_list.pop_front();
	      }

	    // inspect the last cycle
	    if(static_cast<int>(vert_coords.size())>2)
	      if(inspect_face(vert_coords)==false)
		return false;
	  }
      }

    // done
    return true;
  }

  
  // examine whether snapping a vertex to its closest point on a half-edge is legal
  bool is_snap_ok(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const pmp::Halfedge& halfedge)
  {
    // (i) orthogonal projection should lie on the half edge
    // (ii) snapping should result in a valid mesh

    // examine projection
    auto proj_result = projection_on_halfedge(mesh, vertex, halfedge);
    if(proj_result.first==false)
      return false;

    // create a trial mesh to recreate the snap.
    // trial_snap_vertex will be snapped to trial_split_vertex
    pmp::SurfaceMesh trial_mesh;
    pmp::Vertex      trial_snap_vertex;
    pmp::Vertex      trial_split_vertex;
    {
      std::set<pmp::Vertex> vert_set{};
      auto face_circulator = mesh.faces(vertex);
      for(auto face:face_circulator)
	{
	  auto vert_circulator = mesh.vertices(face);
	  for(auto v:vert_circulator)
	    vert_set.insert(v);
	}
      std::map<pmp::Vertex, pmp::Vertex> old_to_new_vert_map{};
      for(auto& old_v:vert_set)
	old_to_new_vert_map.insert({old_v, trial_mesh.add_vertex(mesh.position(old_v))});

      auto jt = old_to_new_vert_map.find(vertex);
      assert(jt!=old_to_new_vert_map.end());
      trial_snap_vertex = jt->second;
      
      // add faces to the mini mesh
      for(auto face:face_circulator)
	{
	  std::vector<pmp::Vertex> vertices{};
	  auto vert_circulator = mesh.vertices(face);
	  for(auto old_v:vert_circulator)
	    {
	      auto it = old_to_new_vert_map.find(old_v);
	      assert(it!=old_to_new_vert_map.end());
	      vertices.push_back(it->second);
	    }
	  trial_mesh.add_face(vertices);
	}

      // split "halfedge" in the new mesh
      auto trial_halfedge = trial_mesh.find_halfedge(old_to_new_vert_map[mesh.from_vertex(halfedge)],
						     old_to_new_vert_map[mesh.to_vertex(halfedge)]);
      assert(trial_mesh.is_valid(trial_halfedge));
      auto split_halfedge = trial_mesh.insert_vertex(trial_mesh.edge(trial_halfedge), proj_result.second);
      trial_split_vertex   = trial_mesh.to_vertex(split_halfedge);
    }

    return is_snap_ok(trial_mesh, trial_snap_vertex, trial_split_vertex);
  }

}
