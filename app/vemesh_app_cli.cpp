// Sriramajayam

/** \file vemesh_app_cli.cpp
 * \brief Implements command-line parsing for vemesh_app.
 * \author Ramsharan Rangarajan
 */

#include "vemesh_app_cli.h"

#include <CLI11.hpp>   // single-header CLI11 provided in external/ (no separate dependency)

#include <iostream>
#include <limits>

std::optional<CLIConfig> parse_cli(int argc, char** argv, int& exit_code)
{
  CLIConfig cfg;

  // Command line options
  CLI::App app{"Agglomerate elements and relax vertices in a mesh"};
  app.footer("Sample usage:	   \
    \n=================== \
    \n(i)   agglomerate elements:  ./vemesh_app -a -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -m stability -v iter \
    \n(ii)  relax vertices:        ./vemesh_app -r -i in_mesh.OFF -o out_dir -n 5 -s 5 -m shape \
    \n(iii) agglomerate and relax: ./vemesh_app --ar -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5 -m stability -v detailed \
    \n(iv)  relax and agglomerate: ./vemesh_app --ra -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5  -m shape\n");

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

  unsigned int seed_val = 0;
  auto opt_seed = app.add_option("-S,--seed", seed_val,
				 "RNG seed for reproducible vertex relaxation (default: nondeterministic)");

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
    exit_code = 0;
    return std::nullopt;
  } 
  catch (const CLI::ParseError &e) {
    exit_code = app.exit(e);
    return std::nullopt;
  }

  // Exactly one optimization mode
  int mode_count = opt_a->count() + opt_r->count() + opt_ar->count() + opt_ra->count();
  if(mode_count != 1)
    {
      CLI::ValidationError err("Optimization mode",
			       "Exactly one optimization flag (-a, -r, --ar, --ra) must be specified");
      exit_code = app.exit(err);
      return std::nullopt;
    }
  
  // Optimization mode
  if (*opt_a)       cfg.mode = CLIConfig::Mode::Agglomerate;
  else if (*opt_r)  cfg.mode = CLIConfig::Mode::Relax;
  else if (*opt_ar) cfg.mode = CLIConfig::Mode::AgglomerateRelax;
  else              cfg.mode = CLIConfig::Mode::RelaxAgglomerate;

  // Optional RNG seed (only set when -S/--seed was supplied)
  if(opt_seed->count() > 0)
    cfg.seed = seed_val;

  // Mesh output mode
  if(output_mode_str=="none")
    cfg.output_mode = CLIConfig::MeshOutputMode::None;
  else if(output_mode_str=="iter")
    cfg.output_mode = CLIConfig::MeshOutputMode::IterationEnd;
  else if(output_mode_str=="detailed")
    cfg.output_mode = CLIConfig::MeshOutputMode::Detailed;

  return cfg;
}
