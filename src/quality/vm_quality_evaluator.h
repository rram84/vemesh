// Sriramajayam

/** \file vm_quality_evaluator.h
 * \brief Definition of the class vm::QualityEvaluator for face/vertex quality evaluations
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <pmp/surface_mesh.h>
#include <utility>
#include <functional>

namespace vm
{
  //! \brief Callable type for evaluating the quality of a polygon.
  //! Accepts any callable (function pointer, lambda, or functor) with
  //! the signature `double(const std::vector<pmp::Point>&)`.
  //!
  //! MeshOptimizer may invoke this callable concurrently from
  //! multiple threads (its quality-evaluation phases are parallelized
  //! with OpenMP).
  //! A custom metric must therefore be thread-safe.
  //! Pure functions of the input coordinates - such as
  //! the built-in vm::quality::vem_stability_ratio and
  //! vm::quality::geom_shape - satisfy this requirement.
  using FaceQualityFn = std::function<double(const std::vector<pmp::Point>&)>;
 
  /**
   * \brief Evaluator for polygonal mesh quality.
   * 
   * This class wraps a face-quality function and allows computing
   * quality metrics for individual faces or vertices of a pmp::SurfaceMesh.
   * The quality of a vertex is defined as the minimum quality among
   * its incident faces:
   * \f[ Q(v) = \min_{1\leq i\leq n}Q(f_i), \f]
   * where \f$f_1,\ldots,f_n\f$ are the faces incident at vertex \f$v\f$ and \f$Q\f$ is the 
   * given face-quality metric.
   * 
   * \sa vm::quality::vem_stability_ratio, vm::quality::geom_shape
   * \ingroup quality
   */
  class QualityEvaluator {

  public:
    /**
     * \brief Construct a QualityEvaluator with a face quality function.
     * \param[in] fq Callable to compute quality given a polygon's vertex coordinates. Copied.
     */
    inline QualityEvaluator(FaceQualityFn fq)
      :fqFunc(fq) {}
    
    //! \brief Destructor
    ~QualityEvaluator() = default;
    
    //! \brief Copy constructor
    QualityEvaluator(const QualityEvaluator&) = default;

    /**
     * \brief Evaluate the quality of a polygon given its vertex coordinates.
     * \param[in] pts Vector of points representing the polygon vertices in CCW order.
     * \pre \p pts lists the polygon vertices in counter-clockwise order.
     * \return Quality value as computed by the provided face-quality function.
     */
    double operator()(const std::vector<pmp::Point>& pts) const;

    /**
     * \brief Evaluate the quality of a face in a pmp::SurfaceMesh.
     * \param[in] face The face whose quality is to be evaluated using the given metric.
     * \param[in] mesh The SurfaceMesh containing the face.
     * \pre Faces in \p mesh have vertices listed in counter-clockwise order.
     * \return Evaluated quality of the face
     */
    double operator()(const pmp::Face& face, const pmp::SurfaceMesh& mesh) const;

    /**
     * \brief Evaluate the quality of a vertex in a pmp::SurfaceMesh.
     * 
     * The vertex quality is defined as the minimum among qualities of all
     * faces incident at the vertex.
     * 
     * \param[in] vertex The vertex whose quality is to be evaluated.
     * \param[in] mesh The pmp::SurfaceMesh object containing the vertex.
     * \pre Faces in \p mesh have vertices listed in counter-clockwise order.
     * \return Evaluated quality of the vertex
     */
    double operator()(const pmp::Vertex& vertex, const pmp::SurfaceMesh& mesh) const;

  private:
     FaceQualityFn fqFunc; //!< The callable holding the face quality metric

  };

}
