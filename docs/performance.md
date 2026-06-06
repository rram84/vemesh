
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

These generators are built together with the unit tests and tutorials when
`BUILD_TESTS=ON` (the default) — there is no separate build option. They are
development/evaluation tools and are not installed.
