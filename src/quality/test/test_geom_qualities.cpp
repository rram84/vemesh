// Sriramajayam

/** \file test_geom_qualities.cpp
 * \brief Unit tests for utility routines vm::quality:geom_shape and vm::quality::geom_min_angle defined in vm_face_qualities.h
 * \ingroup tests
 * \author Ramsharan Rangarajan
 */


#include <vm_face_qualities.h>
#include <pmp/surface_mesh.h>

// non-standard shape
void test_geom_quality_concave_pentagon();

// standard shape

int main()
{
  // test a non-standard shape
  test_geom_quality_concave_pentagon();
}


void test_geom_quality_concave_pentagon()
{
  std::vector<pmp::Point> coords{
    pmp::Point(0.0, 0.0, 0.0),
      pmp::Point(1.0, 0.0, 0.0),
      pmp::Point(0.4, 0.2, 0.0),
      pmp::Point(1.0, 1.0, 0.0),
      pmp::Point(0.0, 1.0, 0.0)};

  const double shape_ref     = 0.473987028388;
  const double min_angle_ref = 18.4349488229; // degrees

  const double shape = vm::quality::geom_shape(coords);
  const double min_angle = vm::quality::geom_min_angle(coords);

  constexpr double tol = 1e-6;

  if (std::abs(shape - shape_ref) > tol)
    {
      std::cerr << "geom_shape failed: "
                << shape << " vs " << shape_ref << "\n";
      std::exit(EXIT_FAILURE);
    }

  if (std::abs(min_angle - min_angle_ref) > tol)
    {
      std::cerr << "geom_min_angle failed: "
                << min_angle << " vs " << min_angle_ref << "\n";
      std::exit(EXIT_FAILURE);
    }
}
