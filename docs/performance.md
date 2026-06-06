
# Performance {#performance}

\note This page is under construction; the evaluation methodology is not yet
documented here.

## Mesh generators

The `performance/` folder contains small driver programs that produce large,
reproducible sets of test meshes for evaluating mesh improvement:

- `generate_embedded_meshes` — embeds a polygonal interface into a background
  triangle mesh over many randomly-perturbed realizations.
- `generate_clipped_meshes` — clips a circular boundary out of a structured quad
  mesh across several refinement levels and realizations.

Both accept a `-S <seed>` option (and print the seed they used) so a mesh set can
be regenerated exactly. Generated meshes are written as VTK for subsequent
improvement with the \ref tutorial_app "vemesh_app" command-line tool.

These generators — together with the \ref tutorial_app "vemesh_app" driver used to
improve the generated meshes — are built alongside the unit tests and tutorials when
`BUILD_TESTS=ON` (the default); there is no separate build option. Because
`vemesh_app` is built here, this configuration requires
[CLI11](https://github.com/CLIUtils/CLI11) for command-line parsing. These are
development/evaluation tools and are not installed.
