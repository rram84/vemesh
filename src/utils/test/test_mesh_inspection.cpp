// Sriramajayam

/** \file test_mesh_inspection.cpp
 * \brief Unit tests for utility routines vm::inspect_face and vm::inspect_mesh defined in vm_utils.h
 * \ingroup tests
 * \author Ramsharan Rangarajan
 */


#include <vm_mesh_inspection.h>
#include <vm_io.h>

// test inspect_face 
void test_inspect_face();

// test failure of inspect_mesh with an empty mesh
void test_inspect_mesh_empty();

// test failure of inspect_mesh with an invalid face
void test_inspect_mesh_invalid_face();

// test failure of inspect_mesh with face having area<=0
void test_inspect_mesh_non_positive_area();

// test failure of inspect_mesh on a mesh with overlapping faces
void test_inspect_mesh_face_overlap();

// test success of inspect_mesh with a valid mesh
void test_inspect_mesh_success(const pmp::SurfaceMesh&);

int main()
{
  // test passing and failing cases of inspect_face 
  test_inspect_face();

  // test failure of inspect_mesh with an empty mesh
  test_inspect_mesh_empty();
  
  // test failure of inspect_mesh with an invalid face
  test_inspect_mesh_invalid_face();

  // test failure of inspect_mesh with a face with area <=0
  test_inspect_mesh_non_positive_area();

  // test failure of inspect_mesh on a mesh with overlapping faces
  test_inspect_mesh_face_overlap();

  // test success of inspect_mesh with a valid mesh
  pmp::SurfaceMesh mesh = vm::read_off("sample_meshes/sorgente_mesh1_40.off");
  test_inspect_mesh_success(mesh);
 
}

// ------ inspect_face ----- //
void test_inspect_face()
{
  // passing case
  std::vector<pmp::Point> coords{
    pmp::Point(0,0,0), pmp::Point(1,0,0), pmp::Point(1,1,0), pmp::Point(0,1,0)};
  
  if (!vm::inspect_face(coords)) {
    std::cerr << "inspect_face failed for valid polygon\n";
    std::exit(EXIT_FAILURE);
  }

  // failing case
  {
    std::vector<pmp::Point> coords{
      pmp::Point(0,0,0), pmp::Point(1,1,0), pmp::Point(0,1,0), pmp::Point(1,0,0)
	};
    
    if (vm::inspect_face(coords)) {
      std::cerr << "inspect_face passed self-intersecting polygon\n";
      std::exit(EXIT_FAILURE);
    }
  }
}


// ------ inspect_mesh, failing case 1 ----- //
void test_inspect_mesh_empty()
{
  pmp::SurfaceMesh mesh;
  vm::ErrorList errors;

  if (vm::inspect_mesh(mesh, vm::MeshInspection::Basic, errors)) {
    std::cerr << "Empty mesh passed Basic inspection\n";
    std::exit(EXIT_FAILURE);
  }

  if (errors.size() != 1) {
    std::cerr << "Expected exactly one error for empty mesh\n";
    std::exit(EXIT_FAILURE);
  }

  if (errors[0].code != vm::InspectionErrorCode::EmptyMesh) {
    std::cerr << "Wrong error code for empty mesh\n";
    std::exit(EXIT_FAILURE);
  }

  if (errors[0].face != -1) {
    std::cerr << "EmptyMesh error must have face == -1\n";
    std::exit(EXIT_FAILURE);
  }
}


// ------ inspect_mesh, failing case 2 ----- //
pmp::SurfaceMesh make_self_intersecting_face_mesh()
{
  pmp::SurfaceMesh mesh;
  auto v0 = mesh.add_vertex(pmp::Point(0.0, 0.0, 0.0));
  auto v1 = mesh.add_vertex(pmp::Point(1.0, 1.0, 0.0));
  auto v2 = mesh.add_vertex(pmp::Point(0.0, 1.0, 0.0));
  auto v3 = mesh.add_vertex(pmp::Point(1.0, 0.0, 0.0));
  mesh.add_face({v0, v1, v2, v3});
  return mesh;
}

void test_inspect_mesh_invalid_face()
{
  auto mesh = make_self_intersecting_face_mesh();
  vm::ErrorList errors;

  if (vm::inspect_mesh(mesh, vm::MeshInspection::FaceGeometry, errors)) {
    std::cerr << "Invalid face passed FaceGeometry inspection\n";
    std::exit(EXIT_FAILURE);
  }

  bool ok = false;
  for (const auto& e : errors) {
    if (e.code == vm::InspectionErrorCode::InvalidFace ||
        e.code == vm::InspectionErrorCode::NonSimpleFace) {

      if (e.face != 0) {
        std::cerr << "Invalid face error should reference face 0\n";
        std::exit(EXIT_FAILURE);
      }

      if (e.face2 != -1) {
        std::cerr << "Invalid face error must not set face2\n";
        std::exit(EXIT_FAILURE);
      }

      ok = true;
    }
  }

  if (!ok) {
    std::cerr << "Expected InvalidFace or NonSimpleFace error\n";
    std::exit(EXIT_FAILURE);
  }
}


// ------ inspect_mesh, failing case 3 ----- //
pmp::SurfaceMesh make_zero_area_face_mesh()
{
  pmp::SurfaceMesh mesh;
  auto v0 = mesh.add_vertex(pmp::Point(0.0, 0.0, 0.0));
  auto v1 = mesh.add_vertex(pmp::Point(1.0, 0.0, 0.0));
  auto v2 = mesh.add_vertex(pmp::Point(2.0, 0.0, 0.0));
  mesh.add_face({v0, v1, v2});
  return mesh;
}

void test_inspect_mesh_non_positive_area()
{
  auto mesh = make_zero_area_face_mesh();
  vm::ErrorList errors;

  if (vm::inspect_mesh(mesh, vm::MeshInspection::FaceGeometry, errors)) {
    std::cerr << "Zero-area face passed inspection\n";
    std::exit(EXIT_FAILURE);
  }

  bool ok = false;
  for (const auto& e : errors) {
    if (e.code == vm::InspectionErrorCode::NonPositiveArea) {

      if (e.face != 0) {
        std::cerr << "NonPositiveArea should reference face 0\n";
        std::exit(EXIT_FAILURE);
      }

      if (e.face2 != -1) {
        std::cerr << "NonPositiveArea must not set face2\n";
        std::exit(EXIT_FAILURE);
      }

      ok = true;
    }
  }

  if (!ok) {
    std::cerr << "Expected NonPositiveArea error\n";
    std::exit(EXIT_FAILURE);
  }
}


// ------- inspect_mesh failing case 4 ------- //
pmp::SurfaceMesh make_overlapping_adjacent_faces_mesh()
{
  pmp::SurfaceMesh mesh;

  // ---- Face 1: unit square ----
  auto v1 = mesh.add_vertex(pmp::Point(1.0, 0.0, 0.0));
  auto v2 = mesh.add_vertex(pmp::Point(1.0, 1.0, 0.0));
  auto v3 = mesh.add_vertex(pmp::Point(0.0, 1.0, 0.0));
  auto v4 = mesh.add_vertex(pmp::Point(0.0, 0.0, 0.0));
  mesh.add_face({v1, v2, v3, v4}); // CCW

  // ---- Face 2: pentagon ----
  auto v5 = mesh.add_vertex(pmp::Point(1.6, 0.0, 0.0));
  auto v6 = mesh.add_vertex(pmp::Point(1.6, 1.0, 0.0));
  auto v7 = mesh.add_vertex(pmp::Point(0.5, 1.9, 0.0));
  auto v8 = mesh.add_vertex(pmp::Point(0.6, 0.6, 0.0)); // inside square
  mesh.add_face({v1, v5, v6, v7, v8, v2}); // CCW, shares edge 1–2

  return mesh;
}

void test_inspect_mesh_face_overlap()
{
  auto mesh = make_overlapping_adjacent_faces_mesh();
  vm::ErrorList errors;

  if (vm::inspect_mesh(mesh, vm::MeshInspection::Adjacency, errors)) {
    std::cerr << "Overlapping faces passed Adjacency inspection\n";
    std::exit(EXIT_FAILURE);
  }

  bool ok = false;
  for (const auto& e : errors) {
    if (e.code == vm::InspectionErrorCode::FaceOverlap) {

      if (e.face != 0 || e.face2 != 1) {
        std::cerr << "FaceOverlap must report faces (0,1)\n";
        std::exit(EXIT_FAILURE);
      }

      ok = true;
    }
  }

  if (!ok) {
    std::cerr << "Expected FaceOverlap error\n";
    std::exit(EXIT_FAILURE);
  }
}


// -------- successful test with inspect_mesh ------ //
void test_inspect_mesh_success(const pmp::SurfaceMesh& mesh)
{
  vm::ErrorList errors;

  if (!vm::inspect_mesh(mesh, vm::MeshInspection::Adjacency, errors)) {
    std::cerr << "Valid mesh failed inspection\n";
    std::exit(EXIT_FAILURE);
  }

  if (!errors.empty()) {
    std::cerr << "Valid mesh produced errors\n";
    std::exit(EXIT_FAILURE);
  }
}

