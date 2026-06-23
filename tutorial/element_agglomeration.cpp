// Sriramajayam

/** \file element_agglomeration.cpp
 * \brief Tutorial-style example for element agglomeration with vemesh.
 * \ingroup tutorial
 */

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_mesh_inspection.h>
#include <vm_io.h>
#include <filesystem>

int main()
{
  // ---- inputs --- 
  // input mesh to improve: should be vtk or off
  const std::string meshfile = "sample_data/sorgente/mesh3_20.off";

  // lower bound for element quality
  const double qepsilon = 0.2;

  // improvement factor to accept an agglomerated element
  const double qfactor = 1.2;

  // directory to write outputs to
  const std::string outdir = "output";
  
  // ---- load the mesh ---- 
  pmp::SurfaceMesh in_mesh = vm::read_off(meshfile);

  // --- optional: inspect the input mesh ---
  vm::MeshInspectionErrors errors;
  bool is_mesh_ok = vm::inspect_mesh(in_mesh, vm::MeshInspection::Adjacency, errors);
  if(!is_mesh_ok) {
    std::cerr << "\nInput mesh failed inspection. \n";
    for(auto& e:errors)
      std::cerr << e << "\n";
    throw std::runtime_error("input mesh failed inspection tests ");
  }
  
  // ---- face quality metric --- 
  const auto face_quality_metric = vm::quality::vem_stability_ratio;

  // ----  quality evaluator ---
  vm::QualityEvaluator QE(face_quality_metric);

  // --- mesh optimizer ---
  vm::MeshOptimizer optimizer(in_mesh);
  auto& mesh = optimizer.get_mesh();    // this is the mesh mutated by the optimizer

  // --- create/clean output directory ---
  namespace fs = std::filesystem;
  const fs::path outpath = fs::path(outdir);
  fs::create_directories(outpath);
  for(const auto& e : fs::directory_iterator(outpath)) {
    if(e.is_regular_file() && e.path().extension() == ".vtk")
      fs::remove(e);
  }

  // --- evaluate and save initial mesh quality ---
  optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
  vm::write_vtk(mesh, outdir+"/input_mesh.vtk");
  vm::write_face_quality_vector(mesh, outdir+"/qvec-input.dat");
  
  // --- callback: save the mesh file after each agglomeration ---
  vm::ProgressCallback callback =
    [outdir](const vm::ProgressInfo& info,
	     const pmp::SurfaceMesh &mesh,
	     const vm::MeshOptimizer &) {
    vm::write_vtk(mesh, outdir+"/mesh-"+std::to_string(info.num_completed)+".vtk");
    return true; };
  
  // --- optimize ---
  int num_agg = optimizer.agglomerate(QE, qepsilon, qfactor, callback);
  std::cout << "Agglomerated " << num_agg << " faces " << std::flush;

  // --- optional: inspect the output mesh ---
  errors.clear();
  is_mesh_ok = vm::inspect_mesh(mesh, vm::MeshInspection::Adjacency, errors);
  if(!is_mesh_ok) {
    std::cerr << "\nOutput mesh failed inspection. \n";
    for(auto& e:errors)
      std::cerr << e << "\n";
    throw std::runtime_error("post-agglomeration mesh failed inspection tests ");
  }

  // evaluate mesh qualities and save file
  optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
  vm::write_vtk(mesh, outdir+"/output_mesh.vtk");
  vm::write_face_quality_vector(mesh, outdir+"/qvec-output.dat");
}
