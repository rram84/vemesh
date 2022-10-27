// Sriramajayam

#include <vm_snap.h>
#include <list>
#include <map>

namespace vm
{
  // snap a vertex to its closest point on a halfedge
  void snap(pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const pmp::Halfedge& halfedge)
  {
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
    auto face_circulator = mesh.faces(vertex);
    for(auto face:face_circulator)
      {
	std::vector<pmp::Vertex> vertices;
	vertices.clear();
	auto vertex_circulator = mesh.vertices(face);
	for(auto v:vertex_circulator)
	  vertices.push_back(v);
	face_vertices.push_back(vertices);
      }

    // remove "vertex"
    mesh.delete_vertex(vertex);
    
    // account for isolated vertices being deleted
    std::map<pmp::Vertex, pmp::Vertex> old_to_new_vert_map{};
    for(auto& it:face_vertices)
      for(auto& v:it)
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
      auto it = old_to_new_vert_map.find(vertex);
      assert(it!=old_to_new_vert_map.end());
      auto jt = old_to_new_vert_map.find(split_vertex);
      if(jt==old_to_new_vert_map.end())
	it->second = split_vertex;
      else
	{
	  split_vertex = jt->second;
	  it->second   = jt->second;
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
