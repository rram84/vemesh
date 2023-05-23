//********************************************************************************
//************************  Poisson Problem 1 (CMAME Paper) **********************
//********************************************************************************

#include <vm_Manager.h>
#include <vm_io.h>
#include <vm_MeshSlicer.h>
#include <vm_SpecialMeshes.h>
#include <vm_face_quality.h>
#include <CLI/CLI.hpp>
#include <set>

// compare pairs of (id, quality)
auto cmp = [](const std::pair<int,double>& a, const std::pair<int,double>& b)
{ return a.second<b.second; };

// print the poorest 10 face qualities
void print_poorest_faces(const std::set<std::pair<int,double>, decltype(cmp)>& qualities)
{
  int count = 0;
  for(auto& it:qualities)
    {
      if(count++>10) break;
      else std::cout << it.second << " ";
    }
}

double level_set_segment(const double* X)
{
  //Line seg from (0,y1) to (1,y2): Equation of the line is; y = y1 + 0.5*(y2 - y1)*(x)
  const double y1 = 0.5;
  const double y2 = 0.5;
  return X[1] - y1 - 0.5*(y2 - y1)*X[0];
}


int main(int argc, char* argv[])
{
  // Command line options
  CLI::App app;

  // Options
  int nx=0, ny=0;
  app.add_option("--nx", nx, "#nodes along x")->required();
  app.add_option("--ny", ny, "#nodes along y")->required();

  // parse
  CLI11_PARSE(app, argc, argv);
  assert(nx>=2 && ny>=2);

  // construct quad mesh
  const double left_cnr[] = {0.,0.};
  const double hx = 1./static_cast<double>(nx-1);
  const double hy = 1./static_cast<double>(ny-1);
  auto rect_mesh = vm::create_rect_mesh(left_cnr, hx, nx, hy, ny);
  vm::write_off(rect_mesh, "rect.off");

  // clipping
  const double phi_eps  = 0.001;    // tolerance for |phi| > phi_eps.
  const double pert_eps = 0.005*hx; // step size for node perturbation
  vm::LevelSetFunction_t lsfunc = level_set_segment;
  auto nadjusted = vm::adjust_mesh_nodes(rect_mesh, phi_eps, pert_eps, lsfunc);
  std::cout << "Adjusted the position of " << nadjusted << " nodes away from the zero level set" << std::endl;
  vm::write_off(rect_mesh, "rect_adjusted.off");
  vm::clip_mesh(rect_mesh, 0.9*phi_eps, lsfunc);
  vm::write_off(rect_mesh, "clipped.OFF");

  // agglomeration
  vm::Manager manager("clipped.OFF");
  auto& mesh = manager.get_mesh();
  vm::write_suku_format(mesh, "clipped");
  
  // mesh quality metric
  vm::MeshFaceQuality_f qfunc = vm::compute_stiffness_based_mesh_face_quality;
  vm::FaceQuality_f     qface = vm::compute_stiffness_based_face_quality;

  // compute face qualities
  std::set<std::pair<int,double>, decltype(cmp)> q_pre(cmp);
  auto f_iterator = mesh.faces();
  for(auto f:f_iterator)
    q_pre.insert({f.idx(), qfunc(mesh,f)});

  // print the poorest 10 face qualities
  std::cout << std::endl << "Poorest 10 qualities: ";
  print_poorest_faces(q_pre);
  std::cout << std::endl;

  // threshold for element quality
  const double qeps = 0.1;
  
  // mesh of faces with bad qualities
  manager.write_bad_faces(std::string("bad_faces.off"), qeps, qfunc);
  
  // agglomerate poor quality faces
  for(auto f:f_iterator)
    if(!mesh.is_deleted(f) && mesh.is_valid(f))
      if(qfunc(mesh,f)<qeps)
	auto success = manager.merge_face(f, qface);

  // print mesh
  manager.write_mesh("merged.OFF");
  vm::write_suku_format(mesh, "merged_agg");
  
  // recompute poorest 10 qualities
  std::set<std::pair<int,double>, decltype(cmp)> q_post(cmp);
  auto f_iterator_post = mesh.faces();
  for(auto f:f_iterator_post)
    q_post.insert({f.idx(), qfunc(mesh,f)});

  // print the poorest 10 face qualities
  std::cout << std::endl << "Poorest 10 qualities: ";
  print_poorest_faces(q_post);
  std::cout << std::endl;

}
