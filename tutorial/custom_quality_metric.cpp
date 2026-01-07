// Sriramajayam

/** \file custom_quality_metric.cpp
 * \brief Tutorial-style example for iterative vertex relaxation with vemesh using a custom quality metric.
 * \ingroup tutorial
 */

#include <vm_mesh_optimizer.h>
#include <vm_io.h>
#include <filesystem>
#include <CLI/CLI.hpp>

// custom quality metric =  normalized min included angle of polygon
double min_angle_metric(const std::vector<pmp::Point>&);

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
  const vm::FaceQualityFn face_quality_metric = min_angle_metric;

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


// compute the angle included by the pair of segments joining three points
/*
 * u    w
 * \  /
 *  v
 */
double compute_included_angle(const pmp::Point& U, const pmp::Point& V, const pmp::Point& W)
{
  // edges
  const double VU[] = {U[0]-V[0], U[1]-V[1]};
  const double VW[] = {W[0]-V[0], W[1]-V[1]};
  
  // measure the angle at vertex V
  const double dot = VU[0]*VW[0] + VU[1]*VW[1];
  const double det = VU[0]*VW[1] - VU[1]*VW[0];
  double angle     = std::atan2(-det, dot);
  if(angle<0.)
    angle += 2.*M_PI;
  
  return angle;
}

// measure quality of a face as the smallest interior angle
// normalized by the included angle for a regular polygon
double min_angle_metric(const std::vector<pmp::Point>& coords)
{
  const int nverts = static_cast<int>(coords.size());
  double min_angle = 2.*M_PI;
  for(int a=0; a<nverts; ++a)
    {
      const auto& Xa = coords[a];
      const auto& Xb = coords[(a+1)%nverts];
      const auto& Xc = coords[(a+2)%nverts];
      
      // angle between edges ab and bc
      const double angle = compute_included_angle(Xa, Xb, Xc);
      
      // track the minimum
      if(angle<min_angle)
	min_angle = angle;
    }

  // normalizing factor
  double norm_angle = M_PI*(nverts-2.0)/nverts;

  // quality measure
  return min_angle/norm_angle;
}
