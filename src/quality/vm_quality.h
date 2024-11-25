// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <utility>
#include <list>
#include <Eigen/Dense>

namespace vm
{
  struct FaceQuality {
    // measure quality of a face as the smallest nonzero eigenvalue of the vem stiffness matrix
    static double stiffness(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
    static double stiffness(const std::vector<pmp::Point>& coords);
    
    // measure quality of a face  as the ratio of the area to the perimeter^2
    static double shape(const pmp::SurfaceMesh& mesh, const pmp::Face& face);

    // measure quality of a face as the smallest included angle
    static double angle(const pmp::SurfaceMesh& mesh, const pmp::Face& face);
  };
  
  struct VertexQuality {

    // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
    // ratio of the area to the perimeter^2
    static double shape(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);

    // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
    // smallest included angle
    static double angle(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);

    // measure quality as the minimum of face qualities around a vertex, with face qualities defined as the
    // smallest nonzero eigenvalue
    static double stiffness(const pmp::SurfaceMesh& mesh, const pmp::Vertex& vert);
  };
  
  // VEM stiffness matrix of a polygon
  Eigen::MatrixXd compute_polygon_stiffness_matrix(const std::vector<pmp::Point>& coords, const double stabilization = 1.0);
}
