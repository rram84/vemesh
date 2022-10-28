// Sriramajayam

#include <vm_move.h>
#include <vm_inspect.h>
#include <limits>

namespace vm
{
  // examine whether a given vertex needs to be moved
  bool needs_move(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex,
		  const double eps_length_ratio)
  {
    assert(mesh.is_valid(vertex) && !mesh.is_deleted(vertex));
    assert(eps_length_ratio<1.);
    
    // examine lengths of half-edges around the vertex
    int nedges = 0;
    double avg_len = 0.;
    double min_len = std::numeric_limits<double>::max();
    auto halfedge_circulator = mesh.halfedges(vertex);
    const auto& Xa = mesh.position(vertex);
    for(auto h:halfedge_circulator)
      {
	assert(mesh.from_vertex(h)==vertex);
	const auto& Xb = mesh.position(mesh.to_vertex(h));
	double len = std::sqrt((Xa[0]-Xb[0])*(Xa[0]-Xb[0]) + (Xa[1]-Xb[1])*(Xa[1]-Xb[1]));
	if(len<min_len)
	  min_len = len;
	avg_len += len;
	++nedges;
      }
    avg_len /= static_cast<double>(nedges);

    // examine the smallest edge length ratio
    const double ratio = min_len/avg_len;
    if(ratio<eps_length_ratio)
      return true;
    else
      return false;
  }


  // identify a feasible point to move a vertex
  std::pair<bool, pmp::Point> feasible_move_point(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const double eps_length_ratio)
  {
    // attempt to move in the direction opposite the shortest halfedge
    double min_len = std::numeric_limits<double>::max();
    pmp::Halfedge shortest_halfedge;
    auto halfedge_circulator = mesh.halfedges(vertex);
    const auto& Xv = mesh.position(vertex);
    double avg_len = 0.;
    int nedges     = 0;
    for(auto h:halfedge_circulator)
      {
	const auto& X = mesh.position(mesh.to_vertex(h));
	double len    = std::sqrt((X[0]-Xv[0])*(X[0]-Xv[0])+(X[1]-Xv[1])*(X[1]-Xv[1]));
	if(len<min_len)
	  {
	    len = min_len;
	    shortest_halfedge = h;
	  }
	avg_len += len;
	++nedges;
      }
    avg_len /= static_cast<double>(nedges);

    // new trial position
    const auto& Xt          = mesh.position(mesh.to_vertex(shortest_halfedge));
    const double unit_vec[] = {(Xt[0]-Xv[0])/min_len, (Xt[1]-Xv[1])/min_len};
    const double lambda     = eps_length_ratio*avg_len;
    const auto& Yv          = pmp::Point(Xv[0]+lambda*unit_vec[0], Xv[1]+lambda*unit_vec[1], 0.);

    // examine whether the new faces are ok
    auto face_circulator = mesh.faces(vertex);
    for(auto face:face_circulator)
      {
	// vertices of this face
	auto face_vertices = mesh.vertices(face);
	std::vector<pmp::Point> vert_coords{};
	for(auto v:face_vertices)
	  if(v==vertex)
	    vert_coords.push_back(Yv);
	  else
	    vert_coords.push_back(mesh.position(v));

	// is this face ok?
	if(inspect_face(vert_coords)==false)
	  return {false, pmp::Point()};
      }

    // success
    return {true, Yv};
  }


  // move a vertex
  void move(pmp::SurfaceMesh& mesh, const pmp::Vertex& vertex, const pmp::Point& X)
  {
    pmp::Point& Y = mesh.position(vertex);
    Y[0] = X[0];
    Y[1] = X[1];
    return;
  }
  
}
