// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>

namespace vm
{
  // compute the visibility polygon
  std::vector<std::pair<double,double>> compute_visibility_polygon(const pmp::SurfaceMesh& mesh,
								   const pmp::Vertex& vertex);

  // sample the visibility polygon
  std::vector<std::pair<double,double>> sample_visibility_polygon(const std::vector<std::pair<double,double>>& vertices, const int num_points);

  // inspect the correctness of a computed visibility polygon
  bool inspect_visibility_polygon(const pmp::SurfaceMesh& mesh,
				  const pmp::Vertex& vertex,
				  const std::vector<std::pair<double,double>>& vis_poly_verts);
}
