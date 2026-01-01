// Sriramajayam

/** \file test_io.cpp
 * \brief Unit tests for mesh I/O routines defined in vm_io.h
 * \author Ramsharan Rangarajan
 */

#include <vm_io.h>
#include <pmp/SurfaceMesh.h>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <filesystem>

// ---------- helpers ---------- //

bool points_close(const pmp::Point& a,
                  const pmp::Point& b,
                  double tol)
{
  return pmp::norm(a - b) <= tol;
}

bool meshes_equal(const pmp::SurfaceMesh& A,
                  const pmp::SurfaceMesh& B,
                  double tol)
{
  if (A.n_vertices() != B.n_vertices()) return false;
  if (A.n_faces()    != B.n_faces())    return false;

  for (auto v : A.vertices())
    if (!points_close(A.position(v), B.position(v), tol))
      return false;
    
  auto fA = A.faces().begin();
  auto fB = B.faces().begin();
  for (; fA != A.faces().end(); ++fA, ++fB)
    {
      std::vector<int> ca, cb;

      for (auto v : A.vertices(*fA)) ca.push_back(v.idx());
      for (auto v : B.vertices(*fB)) cb.push_back(v.idx());

      if (ca != cb) return false;
    }

  return true;
}

bool properties_equal(const pmp::SurfaceMesh& A,
		      const pmp::SurfaceMesh& B)
{
  if (A.has_face_property("domain_id"))
    {
      auto da = A.get_face_property<int>("domain_id");
      auto db = B.get_face_property<int>("domain_id");
      for (auto f : A.faces())
   	if (da[f] != db[f]) return false;
    }

  if (A.has_vertex_property("interface_id"))
    {
      auto ia = A.get_vertex_property<int>("interface_id");
      auto ib = B.get_vertex_property<int>("interface_id");
      for (auto v : A.vertices())
   	if (ia[v] != ib[v]) return false;
    }
  return true;
}

pmp::SurfaceMesh make_test_mesh()
{
  pmp::SurfaceMesh mesh;

  auto v0 = mesh.add_vertex(pmp::Point(0, 0, 0));
  auto v1 = mesh.add_vertex(pmp::Point(1, 0, 0));
  auto v2 = mesh.add_vertex(pmp::Point(1, 1, 0));
  auto v3 = mesh.add_vertex(pmp::Point(0, 1, 0));

  auto f0 = mesh.add_face({v0, v1, v2});
  auto f1 = mesh.add_face({v0, v2, v3});

  auto domain_id = mesh.add_face_property<int>("domain_id", 1);
  auto interface_id = mesh.add_vertex_property<int>("interface_id", 0);

  domain_id[f0] = 2;
  domain_id[f1] = 3;

  interface_id[v0] = 1;
  interface_id[v1] = 2;
  interface_id[v2] = 3;
  interface_id[v3] = 4;

  return mesh;
}

// ---------- tests ---------- //

void test_off_io()
{
  auto mesh0 = make_test_mesh();
  vm::write_off(mesh0, "test.off");
  auto mesh1 = vm::read_off("test.off");

  if (!meshes_equal(mesh0, mesh1, 1.e-8))
    {
      std::cerr << "OFF read/write failed\n";
      std::exit(EXIT_FAILURE);
    }
}

void test_vtk_io()
{
  auto mesh0 = make_test_mesh();
  vm::write_vtk(mesh0, "test.vtk"); 
  auto mesh1 = vm::read_vtk("test.vtk");

  if (!meshes_equal(mesh0, mesh1, 1.e-8))
    {
      std::cerr << "VTK read/write failed\n";
      std::exit(EXIT_FAILURE);
    }

  if (!properties_equal(mesh0, mesh1))
    {
      std::cerr << "VTK read/write failed\n";
      std::exit(EXIT_FAILURE);
    }
}

int main()
{
  test_off_io(); 
  test_vtk_io();

  // clean up
  std::filesystem::remove("test.off");
  std::filesystem::remove("test.vtk");
  
  return EXIT_SUCCESS;
}
