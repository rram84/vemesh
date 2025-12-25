// Sriramajayam

#pragma once

#include <pmp/SurfaceMesh.h>
#include <utility>

namespace vm
{
  using FaceQualityFn = double(*)(const std::vector<pmp::Point>&)>;

  class QualityEvaluator {

  public:
    inline QualityEvaluator(FaceQualityFn fq)
      :fqFunc(fq) {}
    
    //! Destructor
    ~QualityEvaluator() = default;
    
    //! Copy constructor
    QualityEvaluator(const QualityEvaluator&) = default;
    
    //! \brief Evaluate the quality of a face
    double operator()(const std::vector<pmp::Point>& pts) const;

    //! \brief Evaluate the quality of a face in a mesh
    double operator()(const pmp::Face& face, const pmp::SurfaceMesh& mesh) const;

    //! \brief Evaluate the quality of a vertex in a mesh
    double operator()(const pmp::Vertex& vertex, const pmp::SurfaceMesh& mesh) const;

  private:
    const FaceQualityFn fqFunc;
  };

}
