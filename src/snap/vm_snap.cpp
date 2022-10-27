// Sriramajayam

#include <vm_snap.h>
#include <list>
#include <map>
#include <set>

namespace vm
{
  // snap a vertex to its closest point on a halfedge
  void snap(pmp::SurfaceMesh& mesh, std::map<pmp::Vertex, pmp::Vertex>& vertex_map,
	    const pmp::Vertex& vertex, const pmp::Halfedge& halfedge)
  {
    // # vertices
    const int nvertices = mesh.n_vertices();
    
    // vertices of the halfedge
    const auto& vA = mesh.from_vertex(halfedge);
    const auto& vB = mesh.to_vertex(halfedge);
    
    // get the closest point projection of vertex on the halfedge
    auto proj_result = projection_on_halfedge(mesh, vertex, halfedge);
    assert(proj_result.first==true);
    
    // split the halfedge by inserting a new vertex at the projection location
    auto new_halfedge = mesh.insert_vertex(mesh.edge(halfedge), proj_result.second);
    auto split_vertex = mesh.to_vertex(new_halfedge);

    // connectivities of all faces incident at "vertex"
    std::vector<std::vector<pmp::Vertex>> face_vertices{};
    std::set<pmp::Vertex> vert_set{};
    auto face_circulator = mesh.faces(vertex);
    for(auto face:face_circulator)
      {
	std::vector<pmp::Vertex> vertices;
	vertices.clear();
	auto vertex_circulator = mesh.vertices(face);
	for(auto v:vertex_circulator)
	  {
	    vertices.push_back(v);
	    vert_set.insert(v);
	  }
	face_vertices.push_back(vertices);
      }

    // remove "vertex"
    mesh.delete_vertex(vertex);
    {
      auto it = vert_set.find(vertex);
      assert(it!=vert_set.end());
      vert_set.erase(it);
    }
        
    // account for isolated vertices being deleted
    // it is possible that "split_vertex" has also been deleted in the process
    std::map<pmp::Vertex, pmp::Vertex> old_to_new_vert_map{};
    for(auto& v:vert_set)
      if(mesh.is_deleted(v))
	{
	  auto jt = old_to_new_vert_map.find(v);
	  if(jt==old_to_new_vert_map.end())
	    {
	      const auto& X = mesh.position(v);
	      auto new_v    = mesh.add_vertex(X);
	      old_to_new_vert_map.insert({v, new_v});
	    }
	}

    // "vertex" should be mapped to the "split_vertex"
    {
      assert(old_to_new_vert_map.find(vertex)==old_to_new_vert_map.end());
      auto jt = old_to_new_vert_map.find(split_vertex);
      if(jt!=old_to_new_vert_map.end())
	split_vertex = jt->second;
      old_to_new_vert_map.insert({vertex, split_vertex});
    }

    // update map of vertex labels
    for(auto& it:old_to_new_vert_map)
      {
	const auto& curr_v = it.first;
	const auto& new_v  = it.second;

	// if curr_v is a proxy vertex, look up its old label
	// otherwise, use it as is
	auto jt = vertex_map.find(curr_v);
	if(jt==vertex_map.end())
	  vertex_map.insert({new_v,curr_v});
	else
	  {
	    vertex_map.insert({new_v,jt->second});
	    vertex_map.erase(jt);
	  }
      }
    
    // renumber faces
    for(auto& it:face_vertices)
      for(auto& v:it)
	if(mesh.is_deleted(v))
	  {
	    auto jt = old_to_new_vert_map.find(v);
	    assert(jt!=old_to_new_vert_map.end());
	    v = jt->second;
	  }

    // recreate faces in the mesh
    for(auto& vertices:face_vertices)
      {
	// if "split_vertex" is repeated, this face has multiple loops
	int count = std::count(vertices.begin(), vertices.end(), split_vertex);
	if(count==1)
	  mesh.add_face(vertices);
	else
	  {
	    // permute until "split_vertex" appears at the start of the list
	    while(vertices.front()!=split_vertex)
	      std::rotate(vertices.begin(), vertices.begin()+1, vertices.end());

	    // add cycles from "split_vertex" to "split_vertex"
	    std::list<pmp::Vertex> vertex_list(vertices.begin(), vertices.end());
	    std::vector<pmp::Vertex> cycle{};
	    cycle.push_back(vertex_list.front());
	    vertex_list.pop_front();		
	    while(!vertex_list.empty())
	      {
		if(vertex_list.front()!=split_vertex)
		  cycle.push_back(vertex_list.front());
		else
		  {
		    // add this cycle to the mesh
		    if(static_cast<int>(cycle.size())>2)
		      mesh.add_face(cycle);
		     
		    // prepare for the next cycle
		    cycle.clear();
		    cycle.push_back(vertex_list.front());
		  }
		
		vertex_list.pop_front();
	      }

	    // append the last cycle
	    if(static_cast<int>(cycle.size())>2)
	      mesh.add_face(cycle);
	  }
      }

    assert(mesh.n_vertices()==nvertices);
    
    // done
    return;
  }
    
} // vm::
