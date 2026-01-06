// Sriramajayam

/** \file vertex_relaxation.cpp
 * \brief Tutorial-style example for vertex relaxation with vemesh
 * \ingroup tutorial
 */

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_mesh_inspection.h>
#include <vm_io.h>
#include <filesystem>
#include <CLI/CLI.hpp>

int main()
{
  // ---- inputs --- 
  // input mesh to improve: should be vtk or off
  const std::string meshfile = "sample_data/sorgente/mesh3_20.off";

  // lower bound for element quality
  const double qepsilon = 0.2;

  // number of samples points for relaxation
  const int num_samples = 5;

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
  const auto face_quality_metric = vm::quality::geom_shape;

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
  optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
    
  vm::write_vtk(mesh, outdir+"/input_mesh.vtk");
  vm::write_vertex_quality_vector(mesh, outdir+"/qvec-input.dat");
  
  // --- callback: save the mesh file after each vertex perturbation ---
  vm::ProgressCallback callback =
    [outdir](const vm::ProgressInfo& info,
	     const pmp::SurfaceMesh &mesh,
	     const vm::MeshOptimizer &) {
    vm::write_vtk(mesh, outdir+"/mesh-"+std::to_string(info.num_completed)+".vtk");
    return true; };
  
  // --- optimize ---
  int num_relaxed = optimizer.relax(QE, qepsilon, num_samples, callback);
  std::cout << "Relaxed " << num_relaxed << " vertices " << std::flush;

  // --- optional: inspect the output mesh ---
  errors.clear();
  is_mesh_ok = vm::inspect_mesh(mesh, vm::MeshInspection::Adjacency, errors);
  if(!is_mesh_ok) {
    std::cerr << "\nOutput mesh failed inspection. \n";
    for(auto& e:errors)
      std::cerr << e << "\n";
    throw std::runtime_error("post-relaxation mesh failed inspection tests ");
  }
  
  // evaluate mesh qualities and save file
  optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
  optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
  vm::write_vtk(mesh, outdir+"/output_mesh.vtk");
  vm::write_vertex_quality_vector(mesh, outdir+"/qvec-output.dat");
}
