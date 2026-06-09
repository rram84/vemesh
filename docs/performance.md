
# Performance {#performance}

\note This page is under construction; the evaluation methodology is not yet
documented here.

## Mesh generators

The `performance/` folder contains small driver programs that produce large,
reproducible sets of test meshes for evaluating mesh improvement:

- `embed_shapes` — embeds polygonal shapes as interior interfaces into a
  background triangle mesh over many randomly-perturbed realizations.
- `clip_circle` — clips a structured quad mesh to the disk bounded by a circle,
  across several refinement levels and realizations. Under pure-Neumann
  ("do nothing") boundary conditions no degrees of freedom are removed at the
  boundary, so the sliver cut-cells produced along the circle are fully exposed to
  the λ_max/λ₂ conditioning metric (λ₂ = smallest nonzero eigenvalue). This makes
  boundary-clipping a meaningful complement to the interior interface-embedding of
  `embed_shapes`, enriching the test suite with both scenarios.

Both accept a `-S <seed>` option (and print the seed they used) so a mesh set can
be regenerated exactly. Generated meshes are written as VTK for subsequent
improvement with the \ref tutorial_app "vemesh_app" command-line tool.

These generators — together with the \ref tutorial_app "vemesh_app" driver used to
improve the generated meshes — are built alongside the unit tests and tutorials when
`BUILD_TESTS=ON` (the default); there is no separate build option. Because
`vemesh_app` is built here, this configuration requires
[CLI11](https://github.com/CLIUtils/CLI11) for command-line parsing. These are
development/evaluation tools and are not installed.
