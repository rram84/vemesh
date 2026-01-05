// Sriramajayam

/** \file mesh_slicing.cpp
 * \brief Testing utility routines for embedding boundaries/interfaces in meshes
 * \ingroup tests tutorial
 */

#include <vm_tutorial_mesh_slicer.h>
#include <vm_tutorial_rectangle_mesh.h>
#include <vm_io.h>

double level_set_circle(const double* X)
{
  const double center[] {0.5,0.5};
  const double rad = 0.25;
  double Y[] = {X[0]-center[0], X[1]-center[1]};
  double r = std::sqrt(Y[0]*Y[0]+Y[1]*Y[1]);
  return r-rad;
}


int main()
{
  // circle
  vm::tutorial::LevelSetFunction_t lsfunc = level_set_circle;

  // ---- structured rectangle mesh ------ //
  
  // rectangle mesh parameters
  const std::array<double,2> left_cnr{0.,0.};
  const int nx = 15;
  const int ny = 15;
  const double hx = 1./static_cast<double>(nx-1);
  const double hy = 1./static_cast<double>(ny-1);
  auto rect_mesh = vm::tutorial::create_rectangle_mesh(left_cnr, hx, nx, hy, ny);

  // adjust nodes away from the zero level set
  const double pert_eps = 1.e-2;
  const double phi_eps = 1.e-3;
  vm::tutorial::adjust_mesh_nodes(rect_mesh, phi_eps, pert_eps, lsfunc);

  // clip
  auto clipped_mesh = vm::tutorial::clip_mesh(rect_mesh, phi_eps, lsfunc);
  vm::write_vtk(clipped_mesh, "rect-clipped.vtk");

  // embed
  auto embedded_mesh = vm::tutorial::embed_interface(rect_mesh, phi_eps, lsfunc);
  vm::write_vtk(embedded_mesh, "rect-embedded.vtk");

  // ---- random triangle mesh --- //
  auto tri_mesh = vm::read_off("sample_data/random_triangles.OFF");

  // adjust nodes away from zero level set
  vm::tutorial::adjust_mesh_nodes(tri_mesh, phi_eps, pert_eps, lsfunc);

  // clip
  clipped_mesh = vm::tutorial::clip_mesh(tri_mesh, phi_eps, lsfunc);
  vm::write_vtk(clipped_mesh, "tri-clipped.vtk"); 
  
  // embed
  embedded_mesh = vm::tutorial::embed_interface(tri_mesh, phi_eps, lsfunc);
  vm::write_vtk(embedded_mesh, "tri-embedded.vtk");
}
