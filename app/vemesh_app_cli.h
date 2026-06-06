// Sriramajayam

/** \file vemesh_app_cli.h
 * \brief Command-line parsing for vemesh_app.
 *
 * Separates CLI11 option wiring from the optimization driver in
 * vemesh_app.cpp, so that main() reads as the mesh-improvement workflow itself.
 * \author Ramsharan Rangarajan
 */

#pragma once

#include <optional>
#include <string>

// Parsed, validated command-line configuration for vemesh_app.
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

  // optional RNG seed for reproducible vertex relaxation (unset -> nondeterministic)
  std::optional<unsigned int> seed = std::nullopt;

  // output mode
  enum class MeshOutputMode {
    None,           // don't save anything
      IterationEnd,   // save once per iteration
      Detailed        // save after each update / callback
      };
  MeshOutputMode output_mode = MeshOutputMode::None;
};


// Parse and validate argv into a CLIConfig.
//
// Returns the configuration on success. Returns std::nullopt when the program
// should exit immediately without running (e.g. --help was requested, or a
// parse/validation error occurred); in that case \p exit_code is set to the
// value main() should return.
std::optional<CLIConfig> parse_cli(int argc, char** argv, int& exit_code);
