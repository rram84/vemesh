// Sriramajayam

/** \file vm_mesh_inspection.h
 * \brief Defines utilities for mesh inspection
 * \author Ramsharan Rangarajan
 */


#pragma once

#include <vm_utils.h>
#include <string>

namespace vm
{
  /** \brief Levels of mesh inspection.
   *
   * Inspection levels are hierarchical:
   * - Higher levels include all checks from lower levels.
   *
   * \sa inspect_mesh
   * \ingroup utils
   */
  enum class MeshInspection {
    Basic = 0,         //!< Basic mesh sanity checks
      FaceGeometry,    //!< Basic + face geometry checks
      Adjacency        //!< FaceGeometry + adjacency checks
      };
  
  /** \brief Classification of mesh inspection errors.
   *
   * Each error corresponds to a specific failure mode detected during
   * mesh inspection.
   * \ingroup utils
   */
  enum class MeshInspectionErrorCode {
    EmptyMesh,          /**< Mesh has zero vertices, faces, or edges */
      InvalidFace,      /**< Face polygon is invalid (Boost.Geometry validation failed) */
      NonSimpleFace,    /**< Face polygon is not simple (self-intersections) */
      NonPositiveArea,  /**< Face has zero or negative signed area */
      FaceOverlap       /**< Adjacent faces overlap with non-negligible area */
      };


  /** \brief Detailed information about a mesh inspection failure.
   *
   * Stores the error code, affected face indices, and a readable diagnostic message.
   * \ingroup utils
   */
  struct MeshInspectionError {
    MeshInspectionErrorCode code; /**< Error classification */
    int face = -1;                /**< Primary face index (if applicable) */
    int face2 = -1;               /**< Secondary face index (if applicable) */
    std::string message;          /**< Diagnostic message */
  };

  /** \brief Stream output operator for MeshInspectionError.
   *
   * Prints a formatted description of the inspection error, including
   * error code, face indices, and message.
   *
   * \param os Output stream
   * \param e  MeshInspectionError instance
   * \return Reference to the output stream
   * \ingroup utils
   */
  std::ostream& operator<<(std::ostream& os,
			   const MeshInspectionError& e);

  /** \brief Container type for inspection errors.
   * \ingroup utils
   */
  using MeshInspectionErrors = std::vector<MeshInspectionError>;


  /**
 * \brief Inspect a polygonal mesh for geometric and topological validity.
 *
 * Inspection levels are hierarchical:
 * - FaceGeometry includes all Basic checks
 * - Adjacency includes all FaceGeometry and Basic checks
 *
 * The following checks are performed:
 *
 * | Level        | Checks |
 * |-------------|--------|
 * | Basic       | Non-empty mesh, non-zero vertex/edge/face counts |
 * | FaceGeometry| Basic + face validity, simplicity, positive area |
 * | Adjacency   | FaceGeometry + overlap checks between adjacent faces |
 *
 * The inspection exits early on the first failure, regardless of whether
 * an error list is provided.
 *
 * \param[in] mesh   Surface mesh to inspect
 * \param[in] level  Inspection level (hierarchical)
 * \param[out] errors Optional container collecting inspection errors
 *
 * \return True if all requested checks pass, false otherwise
 * \ingroup utils
 */
  bool inspect_mesh(const pmp::SurfaceMesh& mesh,
		    MeshInspection level,
		    std::optional<std::reference_wrapper<MeshInspectionErrors>> errors = std::nullopt);
  
  /** \brief Inspect a single face defined by vertex coordinates.
   *
   * Checks that the polygon defining the face is simple and valid
   *
   * \param[in] coords Ordered list of face vertex coordinates in CCW order
   * \return True if the face polygon is valid and simple, false otherwise
   *
   * \note Does not check face orientation or area sign.
   * \ingroup utils
   */
  bool inspect_face(const std::vector<pmp::Point>& coords);


}
