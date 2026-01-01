// Sriramajayam

/** \file test_quality_evaluator.cpp
 * \brief Unit tests for the class vm::QualityEvaluator
 * \ingroup tests
 * \author Ramsharan Rangarajan
 */

#include <vm_quality_evaluator.h>
#include <vm_face_qualities.h>
#include <pmp/SurfaceMesh.h>
#include <cassert>
#include <cmath>


int main()
{
  constexpr double tol = 1e-8;
  
  // 4-triangle mesh over a square
  pmp::SurfaceMesh mesh;

  // square corners
  auto v0 = mesh.add_vertex(pmp::Point(0,0,0));
  auto v1 = mesh.add_vertex(pmp::Point(1,0,0));
  auto v2 = mesh.add_vertex(pmp::Point(1,1,0));
  auto v3 = mesh.add_vertex(pmp::Point(0,1,0));

  // interior vertex that is off-center
  auto vc = mesh.add_vertex(pmp::Point(0.3,0.4,0));

  // quad vertices attached to right edge
  auto v4 = mesh.add_vertex(pmp::Point(1.6, 0.2, 0.0));
  auto v5 = mesh.add_vertex(pmp::Point(1.4, 1.1, 0.0));
  
  // four triangles
  auto f0 = mesh.add_face({v0, v1, vc});
  auto f1 = mesh.add_face({v1, v2, vc});
  auto f2 = mesh.add_face({v2, v3, vc});
  auto f3 = mesh.add_face({v3, v0, vc});

  // one quad
  auto f4 = mesh.add_face({v1, v4, v5, v2});

  if (!f0.is_valid() || !f1.is_valid() || !f2.is_valid() ||
      !f3.is_valid() || !f4.is_valid())
    {
      std::cerr << "\nFailed to create test mesh faces\n" << std::flush;
      return EXIT_FAILURE;
    }
  
  // evaluator
  vm::QualityEvaluator qe(vm::quality::geom_shape);

  // Face quality: coords vs mesh operator
  for (auto f:mesh.faces())
  {
    std::vector<pmp::Point> coords;
    for (auto v:mesh.vertices(f))
      coords.push_back(mesh.position(v));

    const double q_coords = vm::quality::geom_shape(coords);
    const double q_mesh   = qe(f, mesh);

    if (std::abs(q_coords-q_mesh)>tol)
      {
	std::cerr << "\nFace quality mismatch\n"
		  << "  from coordinates: " << q_coords << "\n"
		  << "  from mesh      : " << q_mesh << "\n"
		  << std::flush;
	return EXIT_FAILURE;
      }
  }

  // Vertex quality = min incident face qualities
  for (auto v : mesh.vertices())
    {
      double qmin = std::numeric_limits<double>::infinity();
      
      for (auto f : mesh.faces(v))
	qmin = std::min(qmin, qe(f, mesh));
      
      const double qv = qe(v, mesh);
      if (std::abs(qv-qmin)>tol)
        {
          std::cerr << "\nVertex quality mismatch\n"
                    << "  computed vertex quality : " << qv << "\n"
                    << "  min incident face value : " << qmin << "\n"
                    << std::flush;
          return EXIT_FAILURE;
        }
    }

  return EXIT_SUCCESS;
}
