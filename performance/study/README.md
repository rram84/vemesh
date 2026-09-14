# Relaxation vs. agglomeration study

Tooling for the Computational Mechanics paper comparing **vertex relaxation** and
**element agglomeration** workflows over many embedded geometries. It answers two
questions:

1. **Which workflow sequence conditions a VEM mesh best** — relax only,
   agglomerate only, or the two interleaved (`--ra`, `--ar`)?
2. **How do the two element-quality measures relate** — the VEM
   `stability_ratio` (which drives the optimizer) vs. the geometric `geom_shape`?

Everything here is study tooling (never on `master`); it builds under
`-DBUILD_TESTS=ON` alongside the other `performance/` tools. Generated data
(`output/`, and the `dataset/` geometry/background meshes) is gitignored.

---

## Pipeline at a glance

```
                          one-time (geometry/)                 per study run (run_geometry.sh)
  .fset feature sets  ─►  fset_to_polygon ─► .dat ┐
  structured grids    ─►  make_background ─► .off ┼─► dataset/ ─► embed_shapes ─► vemesh_app ─► per-op mesh .vtk
  admissibility gates ─►  sampling_check/embed_check       (frozen input)   (perturb)     (-r/-a/--ra/--ar,     │
                                                                                            -v op)              │
                                                                                                                ▼
                                                       globals.csv   ◄── mesh_conditioning (λmax/λ2)  +  mesh_metrics
                                                       perturbed.csv ◄──                                (counts, q per altered face)
                                                                              (each mesh analyzed, then deleted)
```

Two layers:

- **`geometry/` + `dataset/`** — a *one-time* stage that turns raw spline feature
  sets into the frozen study input (527 polygonal geometries + fixed tri/quad
  backgrounds at N=128). Already done; not rerun during the study.
- **top level** — the *study run*: perturb, optimize, measure, discard. Produces
  two scalar CSVs and keeps no meshes.

---

## Running the study (one geometry)

```bash
# build (from repo root)
cmake --build build --target vemesh_app embed_shapes mesh_metrics mesh_conditioning

# run: GEOM stem, background (quad|tri), #realizations, base seed
performance/study/run_geometry.sh 85909 quad 20 12345
# -> performance/study/output/85909_quad/{globals,perturbed}.csv
```

For each realization the driver: embeds one perturbed mesh; then for each driver
metric (`stability`, `shape`) × workflow (`-r -n6`, `-a -n6`, `--ra -n3`,
`--ar -n3`, all = 6 atomic operations) runs `vemesh_app -v op` and analyzes each
per-operation capture the instant it is written, then deletes it. Shared options:
`-q 0.2 -f 1.2 -s 20`. The `base` rows are the unimproved embedded mesh.

Why no staging/batching: conditioning is a fast in-process C++ call
(`mesh_conditioning`), so there is no MATLAB engine start-up to amortize — a mesh
lives only long enough to be measured. This keeps disk bounded regardless of the
realization count.

---

## Output schema

`step` is the linear operation index in **execution order** (`base`=−1): `-r`/`-a`
give `step=0..5`; `--ra` runs r then a each iteration (`step=2k`,`2k+1`), `--ar`
runs a then r. So "quantity vs `step`" is a clean per-workflow x-axis.

`globals.csv` — **one row per captured mesh** (per realization; cheap scalars):

| column | meaning |
|---|---|
| `geom,bg,driver,workflow,real,op,step` | key. `driver`=metric that drove the run |
| `nelems,nverts` | mesh size |
| `n_alt_faces,n_alt_verts` | # elements/vertices altered in that operation |
| `lambda2,lambda_max,cond_ratio` | global VEM conditioning; `cond_ratio = lambda_max/lambda2` |

`altered.csv` — **one row per `(geom,bg,driver,workflow,step)`**, pooled over all
realizations (geometries are *not* merged — that happens at analysis time, so
across-geometry variation stays visible). It stores the **distribution** of the
two metrics over the altered ("perturbed") faces, never individual values:

| column(s) | meaning |
|---|---|
| `geom,bg,driver,workflow,step` | key |
| `n_alt` | total altered faces pooled over realizations |
| `qs_min,qs_max,qg_min,qg_max` | extremes of `q_stability` / `q_geom` |
| `qs_sum,qg_sum,qs_sumsq,qg_sumsq,qsqg_sum` | running sums → exact pooled mean, variance, and the `q_stability`–`q_geom` Pearson correlation |
| `qs_h0..qs_h999` | histogram of `q_stability`, **1000 fixed bins** over [0,1] (bin b = `[b/1000,(b+1)/1000)`) |
| `qg_h0..qg_h999` | histogram of `q_geom`, same bins |
| `s3..s49,s50p` | histogram of `sides` (3…49 explicit, `s50p` = ≥50) |

Everything in `altered.csv` is **additive** (fixed bins, sums, min/max), so
merging realizations or geometries is a column-wise sum; violins / quantiles read
off the pooled histogram, correlation off the pooled sums. Only *altered* faces
contribute: well-shaped elements don't distinguish the two metrics, so the
altered set is the informative sample. (Bin counts `QNB=1000`, `SNB=48` are set in
`mesh_metrics.cpp` and mirrored in the driver's pooling step.)

---

## Tools (top level)

| file | what it does | in → out |
|---|---|---|
| `run_geometry.sh` | study driver (above) | dataset + binaries → 2 CSVs |
| `mesh_metrics.cpp` → `mesh_metrics` | per mesh: counts + per-altered-face `q_stability`,`q_geom`. Recomputes both metrics from geometry (`vm::quality::*`); reads the `altered`/`vertex_altered` flags from the VTK text | `.vtk`, `--tag`, `--emit-faces` → `G,…`/`F,…` CSV lines |
| `vm_study_conditioning.{h,cpp}` | assembles the global k=1 pure-Neumann VEM stiffness from `vm::quality::vem_stiffness_matrix` (the **same** element stiffness the stability ratio uses) and computes `λmax` (Lanczos) and `λ2` (shift-invert) via Spectra | `pmp::SurfaceMesh` → `{λ2,λmax,ratio}` |
| `mesh_conditioning.cpp` → `mesh_conditioning` | thin CLI over the above; a drop-in replacement for the MATLAB `vem_eig` | `.vtk…` → `name,λ2,λmax,ratio` per line |
| `test/test_conditioning.cpp` | unit test (ctest): assembly symmetry, `K·1=0`, VEM patch/consistency, Spectra-vs-dense eigenvalues. Self-contained (no MATLAB) | — |

### Why conditioning is in C++ (not MATLAB)

The global VEM conditioning was historically computed by `performance/matlab/
vem_eig.m`. `mesh_conditioning` reproduces it in C++ so the study needs no MATLAB
and the global cond# is defined from the *same* element stiffness as the element
stability ratio. It was cross-checked against `vem_eig` on identical meshes:
conditioning agrees to ~1e-6 (median), with ~1e-4 only on near-degenerate sliver
triangles (inherent floating-point sensitivity, not a discrepancy in the method).
Those checks live as the self-contained assertions in `test_conditioning`.

---

## One-time geometry prep (`geometry/`)

Produced the frozen `dataset/`; see `dataset/README.md` for provenance. Not part
of a study run.

| file | role |
|---|---|
| `fset_to_polygon.cpp` | oversample `.fset` piecewise-Bézier feature sets → polygonal `.dat` (`--fit` scales to a target box) |
| `make_background.cpp` | structured triangle/quad background meshes over `[-1,1]²` |
| `sampling_check.cpp` | inter-component admissibility: reject a cell crossed by ≥2 distinct loops |
| `embed_check.cpp` | robust embed test under node perturbations |
| `coarsen_geometry.cpp` | morphological close-only coarsening to salvage borderline geometries |
| `trim_geometry.cpp` | loop-drop salvage (superseded by `coarsen_geometry`) |
| `run_embed_sweep.sh` | batch pass/fail sweep that fixed N=128 and the admissible set |

---

## Notes / caveats

- `vemesh_app` is **not** bit-reproducible under a fixed `-S` (OpenMP /
  priority-queue nondeterminism); per-realization meshes vary run-to-run by
  ~0.3%. This is fine for a statistical study (we report distributions over many
  realizations), and it is why *cross-run* conditioning comparisons show a fat
  tail while *same-mesh* comparisons agree to ~1e-6.
- Third-party: `Spectra` (header-only sparse eigensolver) is vendored at
  `external/spectra` and exposed as the `vemesh_spectra` CMake target.
