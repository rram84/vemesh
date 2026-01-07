// Sriramajayam

/** \file embed_boundary.cpp
 * \brief Tutorial-style example embedding a circular boundary in a triangle mesh and subsequent mesh improvement with vemesh
 * \ingroup tutorial
 */

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_io.h>
#include <vm_tutorial_mesh_slicer.h>
#include <filesystem>
#include <CLI/CLI.hpp>

int main()
{
  // ---- mesh and domain inputs --- 

  // input triangle mesh over [-1,1]x[-1,1] 
  const std::string meshfile = "sample_data/tri/bbbb-3.off";
  pmp::SurfaceMesh tri_mesh = vm::read_off(meshfile);
  
  // center and radius of circular domain
  const double circ_center[] = {0.,0.};
  const double circ_radius = 1./std::sqrt(3.);
  const int domain_id = 12;

  // level set function for circular boundary
  vm::tutorial::LevelSetFn ls_circ =
    [circ_center, circ_radius](const double* X) {
    double Y[] = {X[0]-circ_center[0], X[1]-circ_center[1]};
    return std::sqrt(Y[0]*Y[0]+Y[1]*Y[1])-circ_radius;
  };

  // --- output directory ---
  const std::string outdir = "output";
  
  namespace fs = std::filesystem;
  const fs::path outpath = fs::path(outdir);
  fs::create_directories(outpath);
  for(const auto& e : fs::directory_iterator(outpath)) {
    if(e.is_regular_file() && e.path().extension() == ".vtk")
      fs::remove(e);
  }

  // ---- embed the circular boundary in the triangle mesh ---

  // perturb mesh nodes away from the zero level set
  const double phi_tol = 1.e-5; 
  const double pert_dist = 10.*phi_tol;
  vm::tutorial::adjust_mesh_nodes(tri_mesh, phi_tol, pert_dist, ls_circ);

  // embed the circular boundary in the perturbed mesh
  pmp::SurfaceMesh embedded_mesh = vm::tutorial::clip_mesh(tri_mesh, phi_tol, ls_circ);

  // --- algorithmic parameters for agglomeration ---
  
  // number of agglomeration iterations to perform
  const int num_iters = 5;

  // lower bound for element quality
  const double qepsilon = 0.2;

  // improvement factor to accept an agglomerated element
  const double qfactor = 1.2;
  
  // ---- face quality metric --- 
  const auto face_quality_metric = vm::quality::vem_stability_ratio;

  // ----  quality evaluator ---
  vm::QualityEvaluator QE(face_quality_metric);

  // --- mesh optimizer ---
  vm::MeshOptimizer optimizer(embedded_mesh);
  auto& mesh = optimizer.get_mesh();    // this is the mesh mutated by the optimizer

  // --- evaluate and save initial mesh quality ---
  optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
  vm::write_vtk(mesh, outdir+"/embedded_mesh.vtk");
  vm::write_face_quality_vector(mesh, outdir+"/qvec-input.dat");
	
  // --- iteratively optimizer ---
  for(int iter=0; iter<num_iters; ++iter) {
    
    std::cout << "\n Iteration " << iter <<": " << std::flush;
    int num_agg = optimizer.agglomerate(QE, qepsilon, qfactor);
    std::cout << "agglomerated " << num_agg << " faces " << std::flush;

    // evaluate mesh qualities and save file
    optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
    vm::write_vtk(mesh, outdir+"/mesh-iter-"+std::to_string(iter)+".vtk");
    vm::write_face_quality_vector(mesh, outdir+"/qvec-iter-"+std::to_string(iter)+".dat");
  }

  // --- save the final mesh ---
  vm::write_vtk(mesh, outdir+"/output.vtk");
  vm::write_face_quality_vector(mesh, outdir+"/qvec-output.dat");
}
