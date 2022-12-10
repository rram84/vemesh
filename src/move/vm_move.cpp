// Sriramajayam

#include <vm_move.h>
#include <vm_inspect.h>
#include <limits>

namespace vm
{
  // identify a feasible point to move a vertex
  std::pair<bool, std::pair<double,double>> compute_feasible_vertex_position(const pmp::SurfaceMesh& mesh,
									     const pmp::Vertex&      vertex,
									     const double            eps_length_ratio,
									     const double            eps_degrees,
									     const int               num_samples)
  {
    return {false, {0.,0.}};
  }

  /*
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
  min_len           = len;
  shortest_halfedge = h;
  }
  avg_len += len;
  ++nedges;
  }
  avg_len -= min_len;
  --nedges;
  avg_len /= static_cast<double>(nedges);

  // new trial position
  const auto& Xt          = mesh.position(mesh.to_vertex(shortest_halfedge));
  const double unit_vec[] = {(Xt[0]-Xv[0])/min_len, (Xt[1]-Xv[1])/min_len};
  const double lambda     = eps_length_ratio*avg_len;
  const auto& Yv          = pmp::Point(Xv[0]-lambda*unit_vec[0], Xv[1]-lambda*unit_vec[1], 0.);

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
  
  */

}
