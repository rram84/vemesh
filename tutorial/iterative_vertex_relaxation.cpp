// Sriramajayam

/** \file iterative_vertex_relaxation.cpp
 * \brief Tutorial-style example for iterative vertex relaxation with vemesh.
 * \ingroup tutorial
 */

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_io.h>
#include <filesystem>

int main()
{
  // ---- inputs --- 
  // input mesh to improve: should be vtk or off
  const std::string meshfile = "sample_data/sorgente/mesh3_20.off";

  // number of relaxation iterations to perform
  const int num_iters = 5;

  // lower bound for element quality
  const double qepsilon = 0.2;

  // number of sample points per vertex
  const int num_samples = 5;

  // directory to write outputs to
  const std::string outdir = "output";
  
  // ---- load the mesh ---- 
  pmp::SurfaceMesh in_mesh = vm::read_off(meshfile);
  
  // ---- face quality metric --- 
  const auto face_quality_metric = vm::quality::geom_shape;

  // ----  quality evaluator ---
  vm::QualityEvaluator QE(face_quality_metric);

  // --- mesh optimizer ---
  vm::MeshOptimizer optimizer(in_mesh);
  auto& mesh = optimizer.get_mesh();    // this is the mesh mutated by the optimizer

  // --- create/clean output directory
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
	
  // --- iteratively optimizer ---
  for(int iter=0; iter<num_iters; ++iter) {
    
    std::cout << "\n Iteration " << iter <<": " << std::flush;
    int num_relaxed = optimizer.relax(QE, qepsilon, num_samples);
    std::cout << "relaxed " << num_relaxed << " vertices " << std::flush;

    // evaluate mesh qualities and save file
    optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
    optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
    vm::write_vtk(mesh, outdir+"/mesh-iter-"+std::to_string(iter)+".vtk");
    vm::write_vertex_quality_vector(mesh, outdir+"/qvec-iter-"+std::to_string(iter)+".dat");
  }

  // --- save the final mesh ---
  vm::write_vtk(mesh, outdir+"/output.vtk");
  vm::write_vertex_quality_vector(mesh, outdir+"/qvec-output.dat");
}
