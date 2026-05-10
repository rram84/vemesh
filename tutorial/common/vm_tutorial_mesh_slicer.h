// Sriramajayam

#pragma once

#include <pmp/surface_mesh.h>
#include <functional>

namespace vm
{
  namespace tutorial
  {
    /*!
     * \brief Level-set function type.
     *
     * The function takes a pointer to a 2D point \f$(x,y)\f$ and returns the
     * level-set value \f$\phi(x,y)\f$.
     *
     * Negative values indicate the *inner* domain, positive values the *outer*
     * domain.
     *
     * \ingroup tutorial_utils
     */
    using LevelSetFn = std::function<double(const double*)>;

    /**
     * \brief Perturb mesh vertices away from the zero level set
     *
     * This routine ensures that no vertex in the mesh lies too close to the
     * zero level set of a given level-set function. For each vertex whose
     * signed distance satisfies
     * \f[
     *   |\phi(\mathbf{x})| < \varepsilon_{\phi},
     * \f]
     * the vertex position is perturbed by small random displacements in the
     * plane until the condition \f$ |\phi(\mathbf{x})| > \varepsilon_{\phi} \f$
     * is satisfied.
     *
     * The perturbation magnitude is controlled by \p pert_eps. At most 10
     * perturbation attempts are made per vertex. If a suitable displacement
     * cannot be found within this limit, the routine throws a runtime error.
     *
     * \param[in,out] mesh Surface mesh whose vertex positions may be modified.
     *
     * \param[in] phi_eps Minimum admissible absolute value of the level-set function at each
     *            vertex. Must be strictly positive.
     *
     * \param[in] pert_eps Magnitude of each random perturbation applied to a vertex.
     *                     Must be strictly positive.
     *
     * \param[in] ls_func Level-set function \f$ \phi(\mathbf{x}) \f$
     *
     * \note If a vertex cannot be moved sufficiently far from the interface
     *       after 10 perturbation attempts, the function throws a runtime error
     *
     * \note This function modifies the mesh in place and provides no guarantee
     *       of preserving element quality beyond satisfying the level-set
     *       separation criterion.
     *
     * \note Boundary vertices are treated in the same manner as interior vertices
     *       and may also be perturbed.
     *
     * \ingroup tutorial_utils
     */
    void adjust_mesh_nodes(pmp::SurfaceMesh& mesh,
			   const double phi_eps,
			   const double pert_eps,
			   const LevelSetFn &ls_func);


    /**
     * \brief Clip a triangle/quad mesh with the zero level set of a scalar function.
     *
     * This routine modifies a triangle/quad mesh in-place by clipping it with respect
     * to a level-set function \f$\phi(x)\f$. 
     * - Faces lying entirely in the region \f$\phi > 0\f$ are discarded.
     * - Faces while faces intersected by the zero level set are sliced into new elements 
     *   that conform to the interface. 
     * - Only faces lying in the region \f$\phi < 0\f$ are retained.
     *
     * The resulting mesh thus represents the subdomain \f$\phi < 0\f$.
     *
     * The algorithm performs the following steps:
     * - Evaluates the level-set function at all mesh vertices
     * - Identifies faces intersected by the zero level set
     * - Inserts new vertices along cut edges
     * - Slices intersected triangular and quadrilateral faces
     * - Deletes faces lying completely in the outer region
     * - Rebuilds the mesh to remove deleted entities and renumber vertices
     * - Assigns domain_id=0 to all faces in the mesh
     * - Assigns interface_id=1 to boundary nodes, and -1 to remaining nodes
     *
     *
     * \note
     * - It is assumed that all existing mesh vertices satisfy
     *   \f$|\phi(x)| > \texttt{phi_eps}\f$ prior to clipping.
     *   This can be enforced using adjust_mesh_nodes().
     *
     * \warning
     * This routine mutates the input mesh. All face and vertex handles
     * obtained prior to calling this function are invalid after execution.
     * Face/vertex properties in the original mesh are discarded.
     * The returned mesh has the face property `domain_id` and vertex property `interface_id`.
     *
     * \param[in] mesh
     *   Surface mesh to be clipped. The returned mesh only contains the 
     *   \f$\phi < 0\f$ portion of the original mesh.
     *
     * \param[in] phi_eps
     *   Positive tolerance defining a narrow band around the interface.
     *   Vertices are assumed to satisfy \f$|\phi(x)| > \texttt{phi_eps}\f$.
     *
     * \param[in] lsfunc
     *   Level-set function defining the clipping interface.
     *   The interface is given by \f$\phi(x) = 0\f$.
     *
     * \return Clipped mesh
     * - All remaining faces belong to the inner domain.
     * - The mesh contains no deleted entities.
     * - Vertex indices are contiguous.
     * - Face property `"domain_id"` and vertex property `"interface_id"`
     *   are present and consistent with the clipped geometry.
     *
     * \see adjust_mesh_nodes
     * \see embed_interface
     *
     * \ingroup tutorial_utils
     */
    pmp::SurfaceMesh clip_mesh(const pmp::SurfaceMesh &mesh,
			       const double phi_eps,
			       const LevelSetFn &lsfunc);
    

    /**
     * \brief Embed a level-set interface into a surface mesh of triangles/quads.
     * 
     * This routine identifies faces intersected by the zero level-set of the
     * provided `lsfunc` and subdivides them so that the mesh explicitly represents
     * two domains separated by the interface. No part of the mesh is discarded.
     * 
     * The algorithm performs the following steps:
     * - Evaluates the level-set function at all mesh vertices
     * - Identifies faces intersected by the zero level set
     * - Inserts new vertices along cut edges
     * - Slices intersected triangular and quadrilateral faces
     * - Assigns domain_id=0 to faces with |phi|<0, and 1 to faces with |phi|>0
     * - Assigns interface_id=1 to interface nodes, and -1 to remaining nodes
     
     * \param[in] mesh The input mesh of triangles/quads to be modified. Faces intersecting the interface
     *                     will be subdivided, and new vertices may be added along edges.
     * \param[in] phi_eps   Tolerance for identifying nodes close to the interface.
     *                      Vertices with |phi(x)| < phi_eps are considered near the interface.
     * \param[in] lsfunc    Level-set function that defines the interface. Should return
     *                      a signed distance or signed function value at a given point.
     * 
     * \return Mesh with embedded interface.
     * - Face/vertex properties in the original mesh are discarded.
     * - The returned mesh has the face property `domain_id` and vertex property `interface_id`.

     * \note
     * - It is assumed that all existing mesh vertices satisfy
     *   \f$|\phi(x)| > \texttt{phi_eps}\f$ prior to clipping.
     *   This can be enforced using adjust_mesh_nodes().
     * \note Faces must be triangles or quads; other polygon types are not supported.
     * 
     * \warning
     * This routine mutates the input mesh. All face and vertex handles
     * obtained prior to calling this function are invalid after execution.
     *
     * \note The function internally calls `prep_mesh` to compute intersections and
     *       assign interface/domain properties, and `slice_triangle` / `slice_quad` 
     *       to subdivide intersected faces.
     *
     * \see adjust_mesh_nodes
     * \see clip_mesh
     *
     * \ingroup tutorial_utils
     */
    pmp::SurfaceMesh embed_interface(const pmp::SurfaceMesh& mesh,
				     const double phi_eps,
				     const LevelSetFn &lsfunc);

  }
}
