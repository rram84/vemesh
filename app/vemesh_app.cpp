// Sriramajayam

/** \file vemesh_app.cpp
 * \brief Command-line tool for vemesh
 * For detailed usage instructions, see the \ref tutorial_app "Command-line tool" page.
 * \author Ramsharan Rangarajan
 */

// Agglomerate elements and relax vertices in a mesh based on quality

// Flags: pick one
// -a agglomeration
// -r vertex relaxation
// -ar agglomeration + vertex relaxation
// -ra vertex relaxation + agglomeration

// Options
// -i Input polygonal mesh file in OFF/vtk format
// -o output directory, will be cleared if it already exists. will be created otherwise
// -q bound for acceptable element quality
// -n number of iterations to perform
// -s number of vertex samples in case of vertex relaxations
// -f quality improvement factor in case of agglomeration
// -m face quality metric, "stability" = element stability ratio, "shape" = shape quality
// -v optional argument to print meshes after successive merges (within each iteration)

#include <vm_mesh_optimizer.h>
#include <vm_face_qualities.h>
#include <vm_io.h>
#include <CLI/CLI.hpp>

namespace fs = std::filesystem;

struct CLIConfig
{
  // Operation
  enum class Mode {
    Agglomerate,
    Relax,
    AgglomerateRelax,
    RelaxAgglomerate
  };
  Mode mode;
  
  // required options
  std::string meshfile;
  std::string outdir;
  int    num_iters;
  double qepsilon;
  std::string metric;
  
  // operation-specific options
  double qfactor     = 0.;
  int    num_samples = -1;

  // output mode
  enum class MeshOutputMode {
    None,           // don't save anything
    IterationEnd,   // save once per iteration
    Detailed        // save after each update / callback
  };
  MeshOutputMode output_mode = MeshOutputMode::None;
};

// Generate a callback function
vm::ProgressCallback make_mesh_callback(int iter,
					const fs::path &outpath,
					CLIConfig::MeshOutputMode output_mode,
					const std::string descr);

// erase vtk files from output directory
void clean_vtk_outputs(const fs::path &vtk_dir);

int main(int argc, char **argv)
{
  // --------  parse command line options into cfg ---------- //
  CLIConfig cfg;

  // Command line options
  CLI::App app{"Agglomerate elements and relax vertices in a mesh"};
  app.footer("Sample usage:	   \
    \n=================== \
    \n(i)   agglomerate elements:  ./vemesh -a -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -m stability -v iter \
    \n(ii)  relax vertices:        ./vemesh -r -i in_mesh.OFF -o out_dir -n 5 -s 5 -m shape \
    \n(iii) agglomerate and relax: ./vemesh --ar -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5 -m stability -v detailed \
    \n(iv)  relax and agglomerate: ./vemesh --ra -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5  -m shape\n");

  app.set_help_flag("-h,--help", "Print this help message and exit");
  
  // Optimization option
  auto* opt_mode = app.add_option_group("Optimization mode");
  opt_mode->description("Select exactly one optimization mode");
  auto opt_a  = opt_mode->add_flag("-a", "flag to agglomerate elements");
  auto opt_r  = opt_mode->add_flag("-r", "flag to relax mesh vertices");
  auto opt_ar = opt_mode->add_flag("--ar", "flag to agglomerate elements & relax vertices in that order at each iteration");
  auto opt_ra = opt_mode->add_flag("--ra", "flag to relax vertices & agglomerate elements in that order at each iteration");
  //opt_mode->require_option(1);

  // Option dependencies:
  // -i, -o, -n, -q, -m are required by all operations
  // -f is required by agglomerate
  // -s is required by relax
  
  app.add_option("-i", cfg.meshfile, "input mesh file in OFF/vtk format; should exist")
    ->required()
    ->check(CLI::ExistingFile);
  
  app.add_option("-o", cfg.outdir,   "output directory; created if needed; existing VTK files will be removed")
    ->required();

  app.add_option("-n", cfg.num_iters, "number of iterations")
    ->required()
    ->check(CLI::Range(1, std::numeric_limits<int>::max()));

  app.add_option("-q", cfg.qepsilon, "lower bound for acceptable element quality; elements with poorer qualities are marked for improvement")
    ->required()
    ->check(CLI::PositiveNumber);

  app.add_option("-m", cfg.metric, "quality metric \"stability\"=vem_stability_ratio, \"shape\"=geom_shape")
    ->required()
    ->check(CLI::IsMember({"stability", "shape"}));
  
  auto opt_f = app.add_option("-f", cfg.qfactor, "minimum factor of improvement in element quality for agglomeration")
    ->check(CLI::PositiveNumber);
  
  auto opt_s = app.add_option("-s", cfg.num_samples, "number of random samples to generate for vertex relaxation")
    ->check(CLI::PositiveNumber);
  
  std::string output_mode_str = "none";
  app.add_option("-v", output_mode_str,
		 "Mesh output: none | iter | detailed")->check(CLI::IsMember({"none","iter","detailed"}));

  opt_a->needs(opt_f);                 // agglomerate
  opt_r->needs(opt_s);                 // relax
  opt_ra->needs(opt_f)->needs(opt_s);  // agglomerate+relax
  opt_ar->needs(opt_f)->needs(opt_s);

  try {
    app.parse(argc, argv);
  } 
  catch (const CLI::CallForHelp &) {
    std::cout << app.help() << std::endl;
    return 0;
  } 
  catch (const CLI::ParseError &e) {
    return app.exit(e);
  }
  
  int mode_count = opt_a->count() + opt_r->count() + opt_ar->count() + opt_ra->count();
  if(mode_count != 1) {
    throw CLI::ValidationError("Optimization mode", "Exactly one optimization flag must be specified");
  }
  
  // Optimization mode
  if (*opt_a)       cfg.mode = CLIConfig::Mode::Agglomerate;
  else if (*opt_r)  cfg.mode = CLIConfig::Mode::Relax;
  else if (*opt_ar) cfg.mode = CLIConfig::Mode::AgglomerateRelax;
  else              cfg.mode = CLIConfig::Mode::RelaxAgglomerate;

  // Mesh output mode
  if(output_mode_str=="none")
    cfg.output_mode = CLIConfig::MeshOutputMode::None;
  else if(output_mode_str=="iter")
    cfg.output_mode = CLIConfig::MeshOutputMode::IterationEnd;
  else if(output_mode_str=="detailed")
    cfg.output_mode = CLIConfig::MeshOutputMode::Detailed;

  // --------  finished parsing, start optimization ---------- //
  
  // output directory
  const fs::path outpath = fs::path(cfg.outdir) / "vtk/";
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

  // save mesh with qualities
  vm::write_vtk(mesh, (outpath / "input_mesh.vtk").string());

 
  for(int iter=0; iter<cfg.num_iters; ++iter) {

    std::cout << "Mesh improvement iteration " << iter <<":\n" << std::flush;
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
	  optimizer.relax(QE, cfg.qepsilon, cfg.num_samples, callback);
	  break;
	}
      case CLIConfig::Mode::AgglomerateRelax:
	{
	  auto callback_a = make_mesh_callback(iter, outpath, cfg.output_mode, "a");
	  optimizer.agglomerate(QE, cfg.qepsilon, cfg.qfactor, callback_a);
	  auto callback_r = make_mesh_callback(iter, outpath, cfg.output_mode, "a-r");
	  optimizer.relax(QE, cfg.qepsilon, cfg.num_samples, callback_r);
	  break;
	}
      case CLIConfig::Mode::RelaxAgglomerate:
	{
	  auto callback_r = make_mesh_callback(iter, outpath, cfg.output_mode, "r");
	  optimizer.relax(QE, cfg.qepsilon, cfg.num_samples, callback_r);
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
	vm::write_vtk(mesh, outpath.string() + "mesh-iter-" + std::to_string(iter)+".vtk");
      }
  }

  // Save the final mesh
  optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
  optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
  vm::write_vtk(mesh, outpath.string() + "output_mesh.vtk");
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
