// Sriramajayam

/** \file vm_quality_evaluator.h
 * \brief Defines the class vm::QualityEvaluator that performs face and vertex quality evaluations
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <pmp/SurfaceMesh.h>
#include <utility>

namespace vm
{
  //! \brief Function pointer type for evaluating the quality of a polygon
  //! \sa quality::vem_stability_ratio
  //! \sa quality::geom_shape
  //! \sa quality::geom_min_angle
  using FaceQualityFn = double(*)(const std::vector<pmp::Point>&);

  /**
   * \brief Evaluator for polygonal mesh quality.
   * 
   * This class wraps a face-quality function and allows computing
   * quality metrics for individual faces or vertices of a pmp::SurfaceMesh.
   * The quality of a vertex is defined as the minimum quality among
   * its incident faces:
   * \f[ Q(v) = \min_{1\leq i\leq n}Q(f_i), \f]
   where \f$f_1,\ldots,f_n\f$ are the faces incident at vertex \f$v\f$ and \f$Q\f$ is the 
   * given face-quality metric.
   */
  class QualityEvaluator {

  public:
    /**
     * \brief Construct a QualityEvaluator with a face quality function.
     * \param[in] fq Function pointer that computes quality given a polygon's vertex coordinates. Copied.
     */
    inline QualityEvaluator(FaceQualityFn fq)
      :fqFunc(fq) {}
    
    //! \brief Destructor
    ~QualityEvaluator() = default;
    
    //! \brief Copy constructor
    QualityEvaluator(const QualityEvaluator&) = default;

    /**
     * \brief Evaluate the quality of a polygon given its vertex coordinates.
     * \param[in] pts Vector of points representing the polygon vertices.
     * \return Quality value as computed by the provided face-quality function.
     */
    double operator()(const std::vector<pmp::Point>& pts) const;

    /**
     * \brief Evaluate the quality of a face in a pmp::SurfaceMesh.
     * \param[in] face The face whose quality is to be evaluated using the given metric.
     * \param[in] mesh The SurfaceMesh containing the face.
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
     * \return Evaluated quality of the vertex
     */
    double operator()(const pmp::Vertex& vertex, const pmp::SurfaceMesh& mesh) const;

  private:
    const FaceQualityFn fqFunc; //!< Function pointer to the face quality metric

  };

}
