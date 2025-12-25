// Sriramajayam

#pragma once

#include <vm_utils.h>
#include <string>

namespace vm
{
  enum class MeshInspection : unsigned {
    None          = 0,
      Basic         = 1 << 0,  // valence, vertex and face counts, degeneracy
      FaceGeometry  = 1 << 1,  // valid, simple, positive area
      Adjacency     = 1 << 2,  // neighbor overlap
      All           = Basic | FaceGeometry | Adjacency // all of the above
      };

  enum class InspectionErrorCode {
    EmptyMesh,
      InvalidFace,
      NonSimpleFace,
      NonPositiveArea,
      FaceOverlap,
      };

  struct InspectionError {
    InspectionErrorCode code;
    int face = -1;        // primary face
    int face2 = -1;       // optional second face
    std::string message;  // human-readable
  };

  std::ostream& operator<<(std::ostream& os, const InspectionError& e);
  
  using ErrorList = std::vector<InspectionError>;
  
  bool inspect_mesh(const pmp::SurfaceMesh& mesh,
		    MeshInspection level = MeshInspection::All,
		    std::optional<std::reference_wrapper<ErrorList>> errors = std::nullopt);

  bool inspect_face(const std::vector<pmp::Point>& coords);


}
