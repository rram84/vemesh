// Sriramajayam

/** \file vemesh_app.cpp
 * \brief Command-line tool for vemesh
 * For detailed usage instructions, see the \ref tutorial_app "Command-line tool" page.
 * \author Ramsharan Rangarajan
 */

// Agglomerate elements and relax vertices in a mesh based on quality
// Command-line parsing is defined and implemented in vemesh_app_cli.{h,cpp}
// This file is the optimization driver.

#include "vemesh_app_cli.h"

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_io.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace fs = std::filesystem;

// Generate a per-update callback that writes intermediate meshes in detailed mode
vm::ProgressCallback make_mesh_callback(int iter,
					const fs::path &outpath,
					CLIConfig::MeshOutputMode output_mode,
					const std::string descr);

// Remove pre-existing VTK files from the output directory.
void clean_vtk_outputs(const fs::path &vtk_dir);

int main(int argc, char **argv)
{
  // run configuration from command-line arguments
  int exit_code = 0;
  auto maybe_cfg = parse_cli(argc, argv, exit_code);
  if(!maybe_cfg)
    return exit_code;
  const CLIConfig& cfg = *maybe_cfg;

  // threads used by the parallelized quality-evaluation and mesh-inspection loops
#ifdef _OPENMP
  const int nthreads = omp_get_max_threads();
#else
  const int nthreads = 1;
#endif

  // echo the run configuration
  auto mode_name = [](CLIConfig::Mode m) {
    switch(m) {
      case CLIConfig::Mode::Agglomerate:      return "agglomerate (-a)";
      case CLIConfig::Mode::Relax:            return "relax (-r)";
      case CLIConfig::Mode::AgglomerateRelax: return "agglomerate+relax (--ar)";
      case CLIConfig::Mode::RelaxAgglomerate: return "relax+agglomerate (--ra)";
    }
    return "?";
  };
  std::cout << "Run configuration:\n"
	    << "  mode        : " << mode_name(cfg.mode) << "\n"
	    << "  input       : " << cfg.meshfile  << "\n"
	    << "  output dir  : " << cfg.outdir    << "\n"
	    << "  metric      : " << cfg.metric    << "\n"
	    << "  iterations  : " << cfg.num_iters << "\n"
	    << "  min quality : " << cfg.qepsilon  << "\n";
  if(cfg.mode != CLIConfig::Mode::Relax)
    std::cout << "  qfactor     : " << cfg.qfactor << "\n";
  if(cfg.mode != CLIConfig::Mode::Agglomerate) {
    std::cout << "  samples     : " << cfg.num_samples << "\n";
    std::cout << "  seed        : "
	      << (cfg.seed ? std::to_string(*cfg.seed) : "nondeterministic") << "\n";
  }
  std::cout << "  threads     : " << nthreads
	    << " (quality evaluations & mesh inspection)\n" << std::flush;

  // output directory (meshes written directly here, no vtk/ subfolder)
  const fs::path outpath = fs::path(cfg.outdir);
  fs::create_directories(outpath);
  clean_vtk_outputs(outpath);
  
  // Input mesh
  std::string ext = fs::path(cfg.meshfile).extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  pmp::SurfaceMesh in_mesh;
  if(ext==".off")
    in_mesh = vm::read_off(cfg.meshfile);
  else if(ext==".vtk")
    in_mesh = vm::read_vtk(cfg.meshfile);
  else
    throw std::runtime_error("Expected mesh format to be off/vtk, given "+cfg.meshfile);
  
  // Mesh optimizer
  vm::MeshOptimizer optimizer(in_mesh);
  const auto& mesh = optimizer.get_mesh();

  // Face quality metric
  const vm::FaceQualityFn face_quality_metric =
    (cfg.metric=="stability") ? vm::quality::vem_stability_ratio : vm::quality::geom_shape;
	    
  // Quality evaluator
  vm::QualityEvaluator QE(face_quality_metric);
  
  // initial mesh quality
  optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
  optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
  vm::write_vtk(mesh, (outpath / "input_mesh.vtk").string());

  // improvement iterations
  for(int iter=0; iter<cfg.num_iters; ++iter) {

    std::cout << "Mesh improvement iteration " << iter <<":\n" << std::flush;

    // relaxation seed. nullopt when -S is not given -> nondeterministic
    const std::optional<unsigned int> rseed =
      cfg.seed ? std::optional<unsigned int>(*cfg.seed + static_cast<unsigned int>(iter))
               : std::nullopt;
    switch(cfg.mode)
      {
      case CLIConfig::Mode::Agglomerate:
	{
	  auto callback = make_mesh_callback(iter, outpath, cfg.output_mode, "a");
	  optimizer.agglomerate(QE, cfg.qepsilon, cfg.qfactor, callback);
	  break;
	}
      case CLIConfig::Mode::Relax:
	{
	  auto callback = make_mesh_callback(iter, outpath, cfg.output_mode, "r");
	  optimizer.relax(QE, cfg.qepsilon, cfg.num_samples, callback, rseed);
	  break;
	}
      case CLIConfig::Mode::AgglomerateRelax:
	{
	  auto callback_a = make_mesh_callback(iter, outpath, cfg.output_mode, "a");
	  optimizer.agglomerate(QE, cfg.qepsilon, cfg.qfactor, callback_a);
	  auto callback_r = make_mesh_callback(iter, outpath, cfg.output_mode, "a-r");
	  optimizer.relax(QE, cfg.qepsilon, cfg.num_samples, callback_r, rseed);
	  break;
	}
      case CLIConfig::Mode::RelaxAgglomerate:
	{
	  auto callback_r = make_mesh_callback(iter, outpath, cfg.output_mode, "r");
	  optimizer.relax(QE, cfg.qepsilon, cfg.num_samples, callback_r, rseed);
	  auto callback_a = make_mesh_callback(iter, outpath, cfg.output_mode, "r-a");
	  optimizer.agglomerate(QE, cfg.qepsilon, cfg.qfactor, callback_a);
	  break;
	}
      }

    // output at the end of the iteration
    if(cfg.output_mode == CLIConfig::MeshOutputMode::IterationEnd)
      {
	optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
	optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
	vm::write_vtk(mesh, (outpath / ("mesh-iter-" + std::to_string(iter) + ".vtk")).string());
      }
  }

  // Save the final mesh
  optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
  optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
  vm::write_vtk(mesh, (outpath / "output_mesh.vtk").string());
}


// Generate a callback function
vm::ProgressCallback make_mesh_callback(int iter,
					const fs::path &outpath,
					CLIConfig::MeshOutputMode output_mode,
					const std::string descr)
{
  // If output mode is "None" or "IterationEnd", return a callback that does nothing
  if(output_mode == CLIConfig::MeshOutputMode::None || output_mode==CLIConfig::MeshOutputMode::IterationEnd)
    return [](const vm::ProgressInfo&, const pmp::SurfaceMesh&, const vm::MeshOptimizer&) { return true; };
  
  // Output mode is detailed. generate a callback that writes VTK files
  return [iter, outpath, descr](const vm::ProgressInfo& info,
				const pmp::SurfaceMesh &mesh,
				const vm::MeshOptimizer &)
    {
      std::string fname;
      fname = (outpath / ("mesh-iter-" + std::to_string(iter) + "-" + descr + "-" + std::to_string(info.num_completed) + ".vtk")).string();
      vm::write_vtk(mesh, fname);
      return true; 
    };
}

// erase vtk files from output directory
void clean_vtk_outputs(const fs::path &vtk_dir)
{
  if(!fs::exists(vtk_dir)) return;

  for(const auto& e : fs::directory_iterator(vtk_dir)) {
    if(e.is_regular_file() && e.path().extension() == ".vtk")
      fs::remove(e);
  }
}
