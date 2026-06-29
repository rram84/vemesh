
# VEMesh in practice {#performance}

This page provides representative snapshots of the performance of
VEMesh when working with real meshes in scenarios we expect the
library to be used in. 

## VEM stiffness matrix
In keeping with the rationale of the library, we emphasize the
improvement in conditioning of the stiffness matrix in the VEM. For
demonstration purposes, we
choose the Laplacian operator,  corresponding to the usual bilinear form
\f$a(u,v)=\int_\Omega \nabla u\cdot\nabla v\,\mathrm{d}x\f$.  The
lowest-order nodal VEM replaces it by a computable
surrogate \f$a_h(\cdot,\cdot)\f$ for which the stiffness matrix is given
by
\f[
  K_{ij} \;=\; a_h(\varphi_j,\varphi_i)
        \;=\; \sum_{E}\left[\;
            \int_E \nabla\Pi^{\nabla}_E\varphi_j \cdot \nabla\Pi^{\nabla}_E\varphi_i \,\mathrm{d}x
            \;+\; S^E\!\big((I-\Pi^{\nabla}_E)\varphi_j,\,(I-\Pi^{\nabla}_E)\varphi_i\big)
        \;\right],
\f]
where \f$\Pi^{\nabla}_E\f$ is the element-wise elliptic projection onto linear
polynomials and \f$S^E\f$ is the standard VEM stabilization factor
that acting on the part of the
basis the projection cannot represent. In all the numerical
experiments discussed below, we set \f$S^E=1\f$. The
summation over \f$E\f$ in the expression represents the usual assembly of
the sparse stiffness matrix. We presume "do nothing" boundary
conditions (i.e., a pure Neumann
problem), which renders \f$K\f$ positive *semi*-definite due to the constant
function being in the null-space of the matrix. Therefore, we measure the
condition number of \f$K\f$ as 
\f[
  \mathrm{cond}(K) \;=\; \frac{\lambda_{\max}}{\lambda_2},
\f]
where \f$\lambda_1,\lambda_2,\ldots,\lambda_{\max}\f$ represent
eigenvalues ordered in increasing order of magnitude. We use \f$\lambda_2\f$ because the first eigenvalue in the list is
\f$\lambda_1=0\f$(corresponding to the eigenvector in the nullspace). 

The stiffness matrix and its eigenvalues are computed independently of
the VEMesh library by a small MATLAB kernel (`performance/matlab/`)
that is based on O. Sutton's "Virtual element method in 50 lines of
MATLAB" \cite sutton2017vem.

## Three case studies
We present three case studies encompassing scenarios for which VEMesh
has been envisioned. 

1. \ref performance_element "**Element improvement**" uses
   unstructured polygonal meshes from \cite sorgente2022role to
   benchmark the element agglomeration and vertex relaxation
   operations provided by VEMesh against the literature.
   
2. \ref performance_interfaces "**Embedded interfaces**" embeds
   interfaces in non-conforming background triangle meshes and
   examines improvement in conditioning  with   VEMesh.
   
3. \ref performance_boundaries "**Clipped boundaries**" embeds
   boundaries in structured quad meshes and examines improvement in
   conditioning with VEMesh.

All three studies focus on dealing with pooly shaped polygonal
elements, which have been frequently associated with ill-conditioning
in the VEM.  In each study, we highlight key takeaways that serve to
justify the teachnical aspects of VEMesh and shed insights on the
relationship between element shapes and ill-conditioning.


## 1. Mesh improvement vs Matrix Conditioning {#performance_element}

The first study works directly on unstructured polygonal meshes.  The
study by Sorgente et al. in \cite sorgente2022role considers a
collection of five progressively refined random triangulations of a
square domain (mesh1, mesh2,...,mesh5). The reference provides
algorithms that coarsen these meshes to improve their qualities in a
certain sense by extremizing a global objective to achieve a specific
reduction in element count. VEMesh improves these triangulations using
local agglomeration and relaxation operations. Unlike \cite
sorgente2022role, VEMesh does not target a specific reduction in
element count during agglomeration; instead, it operates on elements
whose qualities fall below a specified threshold.

### a. VEMesh improvement meshes
Fig 1 shows the coarsest triangulation mesh1 considered in the
experiment. Its improvement by Sorgente et al, retaining 40% and 20%
of the elements, are the meshes labeled mesh1_40 and mesh1_20. We
improve mesh1 with VEMesh in three ways: using element agglomeration
alone, through vertex relaxation alone, and by combining alternating
agglomeration and relaxation. The resulting three meshes are labeled
mesh1_agglomerate, mesh1_relax, mesh1_agglomerate_relax. Each of these
VEMesh runs employs a total of 6 iterations, the element stability
ratio as the quality metric, and a quality threshold of 0.2. As
evident from the figure, a key distinction in VEMesh compared to the
work of Sorgente et al is that both element agglomeration and vertex
relaxation in VEMesh preserve the set of nodes, and hence the degrees
of freedom.

\htmlonly
<div class="image"><img src="mesh1_all.png" alt="mesh1 and its improvements"/><div class="caption">Figure 1. The coarsest benchmark triangulation mesh1, the Sorgente coarsenings retaining 40% and 20% of the elements (mesh1_40, mesh1_20), and the three VEMesh improvements (mesh1_agglomerate, mesh1_relax, mesh1_agglomerate_relax). VEMesh preserves the node set, and hence the degrees of freedom.</div></div>
\endhtmlonly


<br/>

### b. VEMesh improvement meshes optimized by Sorgente et al.
Fig 2a plots the condition numbers of 30 meshes as a function of the
element count on a log-log scale. These include:
- The baseline triangulations (label: _baseline)
- Meshes produced by the algorithm of Sorgente et al. to improve the
baseline while retaining 40% of elements (label: _40)
- Meshes produced by the algorithm of Sorgente et al. to improve the
baseline while retaining 20% of elements (label: _20)
- Meshes produced by 6 iterations of element agglomeration in VEMesh with a
   quality threshold of 0.2 (label: agglomerate)
- Meshes produced by 6 iterations of vertex relaxation in VEMesh with
   a quality threshold of 0.2 (label: relax)
- Meshes produced by 3 iterations of alternating element
   agglomeration and vertex relaxation (label: agglomerate_relax)


\htmlonly
<div class="image"><img src="sorgente_cases_full.png" alt="Condition
number vs. element count, and sorted element quality vectors for
mesh5"/><div class="caption">Figure 2. <b>(a, left)</b> Condition
number vs. element count (log&ndash;log) for the baseline
triangulations and their improvements &mdash;  Sorgente et al's 40%
and 20% coarsenings, and the three VEMesh variants.  All VEMesh
variants improve on the baseline, and agglomeration gains at least an
order of magnitude over the alternatives. <b>(b, right)</b> Sorted
element stability-ratio vectors (log&ndash;log) for mesh5 and its
improved counterparts; agglomeration provides the most benefit in
improving the poorest elements, which translates to improvement in
matrix conditioning improvement.</div></div>
\endhtmlonly



The plot shows that all variants of VEMesh improve the condition
numbers over the baseline case. The result of vertex relaxation with
VEMesh is comparable with those determined by Sorgente et al. However,
element agglomeration with VEMesh results in noticable improvement.
The condition numbers of the meshes incorporating agglomeration are in
fact at least an order of magnitude better than the alternatives.

The reason underlying the improvement in condition numbers in VEMesh
is revealed by examining the element quality vectors shown in Fig
2b. The figure shows the data for the most refined mesh (mesh5) and
its improved counterparts. The log-log scale used in the plot helps
highlight the significance of elements with poor qualities. We find
that element agglomeration dramatically improves the qualities of the
poorest elements in the mesh, and this directly aids in improving the
conditioning number in the VEM. It is critical that element qualities
in the plot refer to the stability ratio, rather than a metric
correlated to polygon shapes.


Next, we demonstrate the VEMesh in fact improves the meshes optimized
by Sorgente et al. To this end, we run VEMesh not on the baseline
meshes, but on the optimized 40% coarsenings of Sorgente et al, i.e.,
the meshes mesh1_40,...,mesh5_40.  It is not evident a priori that
VEMesh will improve these meshes.  Fig 3 plots the condition numbers
of 20 meshes. These include:
1. The new baseline cases.
2. Meshes produced by 6 iterations of element agglomeration in VEMesh
   on the new baseline meshes,  with a quality threshold of 0.2
   (label: agglomerate)
3. Meshes produced by 6 iterations of vertex relaxation in VEMesh
   on the new baseline meshes,  with a quality threshold of 0.2
   (label: relax)
4. Meshes produced by 3 iterations of alternating element
   agglomeration and vertex relaxation in VEMesh
   on the new baseline meshes,  with a quality threshold of 0.2
   (label: agglomerate_relax)


\htmlonly
<div class="image"><img src="sorgente_cases_40.png" alt="Condition number vs. element count for the Sorgente 40% coarsenings and their VEMesh improvements"/><div class="caption">Figure 3. Condition number vs. element count (log&ndash;log) for the Sorgente 40% coarsenings used here as the new baseline, and their VEMesh improvements (agglomeration, relaxation, alternating). VEMesh improves on the already-optimized Sorgente meshes in nearly every case. The gray markers show VEMesh driven by a shape ratio instead of the stability ratio; the gain is markedly smaller, underscoring the stability ratio as the more effective quality metric.</div></div>
\endhtmlonly

We see from the plot that in nearly every case, VEMesh improves the
result of Sorgente et al. This is despite the coarsening in \cite
sorgente2022role being optimal in a specific sense. Hence, the
improvement of these meshes with VEMesh is a non-trivial outcome. The
localized nature of the agglomeration and relaxation operations in
VEMesh compared to the global (combinatorial) optimization in \cite
sorgente2022role makes the result even more significant. 


### c. Element stability ratio is key to improving condition numbers
We attribute the improvement in condition numbers with VEMesh to the
choice of element quality metric. The element stability ratio is more
directly correlated with the conditioning number than shape-related
metrics. To demonstrate this point quantitatively, Fig 3 additionally
plots VEMesh's results when using the shape ratio rather than the
stability ratio for as the element quality metric. These results are
plotted in gray. We see that the improvement in condition nmbers using
the shape ratio is far less dramatic than the stability ratio.

Fig 4 further validates the intuition that the element stability ratio
is well-correlated with the global conditioning number. There, we plot
the minimum element stability ratio in a mesh against the
corresponding matrix conditioning number for each of the 30 meshes in
Fig 1. To account for the influence of the mesh size on the condition
number, we divide the latter by the number of vertices \f$({\rm
N_verts})\f$ in each mesh; this is due to the expected scaling of the
conditioning number with \f$h^2\f$, with \f$h\f$ being a
representative mesh size parameter. The correlation observed in the
log-log plot justifies why improving elements with the poorest
stability ratios improves the condition number.


STOP HERE

\htmlonly <div class="image"><img src="sorgente_cases_full.svg"
alt="Conditioning vs. element count for the full meshes"/><div
class="caption">Conditioning vs. element count: each VEMesh variant
against the baseline mesh and the Sorgente 20%/40%
coarsenings.</div></div> \endhtmlonly

staged under
`sample_data/sorgente/`) — and asks how much VEMesh improves their conditioning,
and how that improvement compares to simply using a coarser mesh.

Each base mesh is improved by the three VEMesh variants and placed alongside two
points of reference: the original (full) mesh, and the **Sorgente coarsenings**
that retain roughly 20% and 40% of the elements (the meshes).
Plotting condition number against element count on log–log axes
(`plot_sorgente.sh`) then shows whether VEMesh buys conditioning improvement at a
comparable or better "cost" in element count than literature coarsening — and
crucially that it does so *without changing the degree-of-freedom count*, since
agglomeration alters element connectivity but preserves vertices.

\htmlonly
<div class="image"><img src="sorgente_cases_40.svg" alt="The same comparison starting from the 40%-retained mesh"/><div class="caption">The same comparison starting from the 40%-retained mesh.</div></div>
\endhtmlonly

This study is also where the **shape-vs-stability** comparison lives. For a given
mesh the per-element quality vectors under the stability metric are compared
across strategies, and assembled conditioning is plotted against the worst
element's stability ratio. The takeaway is that the stability ratio — the
criterion VEMesh agglomerates on — moves in lock-step with assembled
conditioning, which is exactly why a *shape-agnostic*, stability-driven criterion
is preferable to a geometric shape metric (\cite vem-cmame).

\htmlonly
<div class="image"><img src="quality_mesh5_full.svg" alt="Sorted per-element stability-ratio vectors for each strategy on the full mesh"/><div class="caption">Sorted per-element stability-ratio vectors for each strategy on the full mesh: the improvement primitives lift the poorest elements.</div></div>
\endhtmlonly

\htmlonly
<div class="image"><img src="quality_mesh5_40.svg" alt="The same per-element quality comparison on the 40%-retained mesh"/><div class="caption">The same per-element quality comparison on the 40%-retained mesh.</div></div>
\endhtmlonly

\htmlonly
<div class="image"><img src="compare_stability_vs_shape_40.svg" alt="Stability metric vs. geometric shape metric on the 40%-retained mesh"/><div class="caption">Stability metric vs. geometric shape metric on the 40%-retained mesh: the two rank elements differently.</div></div>
\endhtmlonly

\htmlonly
<div class="image"><img src="correlation_scaledcond_vs_minq.svg" alt="Scale-corrected conditioning vs. the worst element stability ratio, pooled across meshes"/><div class="caption">Scale-corrected conditioning vs. the worst element's stability ratio, pooled across meshes: the stability metric tracks assembled conditioning.</div></div>
\endhtmlonly





Why conditioning, and only conditioning? Because the companion question — whether
reshaping the mesh degrades the solution — is already settled. The CutVEM study
\cite vem-cmame shows that the agglomeration and relaxation modifications applied here
leave the method's **accuracy and convergence properties unchanged**; what remains to
be shown, and what these snapshots show, is how much they improve conditioning.

\par The three snapshots
Every study below drives VEMesh through the \ref tutorial_app "vemesh_app"
command-line tool in three variants — agglomeration only, relaxation only, and
alternating agglomerate + relax — against the unimproved baseline, and reports the
conditioning defined above.


The interface and boundary studies generate **many randomly-perturbed realizations**
of each configuration and report the *distribution* of conditioning over them, so the
conclusions rest on statistics rather than a single lucky or unlucky cut. Algorithm
parameters — iteration counts, quality thresholds, agglomeration growth factors — are
held at sensible per-study values throughout; this is an evaluation of outcomes, not a
parametric tuning exercise.


## 2. CutVEM: meshes with embedded interfaces {#performance_interfaces}

The second study targets the setting CutVEM was built for: a background mesh into
which an interface is **embedded**, cutting the elements it crosses into slivers.
The `embed_shapes` generator embeds a polygonal interface (from
`sample_data/shapes/`) into a fixed background triangle mesh
(`sample_data/tri/`), producing the cut cells that wreck conditioning.

To make the result statistically meaningful rather than dependent on one lucky or
unlucky cut, `run_shapes.sh` generates **many randomly-perturbed embeddings** of
each interface and improves every one. `analyze_shapes.sh` computes the
conditioning of every resulting mesh, and `plot_shapes.py` shows, per interface
shape, the *distribution* of conditioning over the realizations as grouped violin
plots — one violin per variant — so the spread, not just the mean, is visible.
A companion correlation figure pools all realizations and variants to show worst
element stability ratio against (scale-corrected) conditioning.

\htmlonly
<div class="image"><img src="shapes_condition_dist.svg" alt="Conditioning distribution over random embeddings, per interface shape"/><div class="caption">Conditioning distribution over random embeddings, per interface shape, with one violin per variant.</div></div>
\endhtmlonly

## 3. CutVEM: meshes clipped by boundaries {#performance_boundaries}

The third study is the boundary counterpart of the second. Instead of embedding
an interface in the interior, `clip_circle` **clips** a structured quadrilateral
mesh against a circular boundary, producing sliver cut cells along the boundary
itself. Under pure-Neumann boundary conditions no degrees of freedom are removed
at the boundary, so these boundary slivers are fully exposed to the
\f$\lambda_{\max}/\lambda_2\f$ metric — making boundary clipping a genuine
complement to the interior interface-embedding of Section 2.

The clipping is swept over several **refinement levels** (level 0 has \f$h=0.2\f$;
each level halves \f$h\f$). As with the embedded-interface study, many randomly
perturbed clippings are generated per level (`run_circle.sh`), analyzed
(`analyze_circle.sh`) and plotted as per-level violin distributions
(`plot_circle.py`). Because bare-stiffness conditioning grows as \f$h^{-2}\f$ with
refinement (\f$\lambda_2 \sim h^2\f$), the distribution figure scales the
condition number by \f$h^2\f$, removing that purely geometric drift so that the
*improvement* across levels is comparable on one footing.

\htmlonly
<div class="image"><img src="circle_condition_dist.svg" alt="Conditioning distribution over random clippings, per refinement level"/><div class="caption">Conditioning distribution over random clippings, per refinement level, with one violin per variant. The condition number is scaled by h^2 to remove the refinement drift.</div></div>
\endhtmlonly

## Running the tests {#performance_running}

The three studies share a common three-stage pipeline — **generate &
improve** → **analyze** → **plot** — kept as separate steps so the (slow) mesh
generation and the (MATLAB-dependent) eigenvalue analysis are not repeated when
only the plots change. Generated meshes are kept on disk under
`performance/output/`; nothing is erased between stages.

The mesh generators (`embed_shapes`, `clip_circle`) and `vemesh_app` are built
alongside the unit tests and tutorials when `BUILD_TESTS=ON` (the default); there
is no separate build option, and because `vemesh_app` is built here this
configuration requires [CLI11](https://github.com/CLIUtils/CLI11). The generators
accept a `-S <seed>` option (and print the seed they used) so any mesh set can be
regenerated exactly. The analysis stage requires MATLAB or Octave on the `PATH`;
the plotting stage requires Python with matplotlib (`*.py`) or gnuplot
(`*.sh`). These are development/evaluation tools and are not installed.

| Study | Generator | Generate & improve | Analyze (eigen) | Plot |
|-------|-----------|--------------------|-----------------|------|
| \ref performance_element "Element improvement" | — (`sample_data/sorgente/`) | `run_sorgente.sh` | `analyze_sorgente.sh` | `plot_sorgente.sh` |
| \ref performance_interfaces "Embedded interfaces" | `embed_shapes` | `run_shapes.sh` | `analyze_shapes.sh` | `plot_shapes.py` |
| \ref performance_boundaries "Clipped boundaries" | `clip_circle` | `run_circle.sh` | `analyze_circle.sh` | `plot_circle.py` |

A typical run, from the build tree, is:

```sh
# from <build>/performance, after `cmake --build` has staged the scripts
./run_shapes.sh        # generate perturbed embeddings + improve (writes output/shapes/)
./analyze_shapes.sh    # assemble VEM stiffness + eigenvalues   (writes per-shape CSVs)
python3 plot_shapes.py # violin distributions + correlation figure
```

The other two studies follow the same pattern with their respective scripts. Each
script carries a configuration block at its top (meshes/shapes/levels, variant
option strings, realization counts, parallel job count, and the MATLAB/Octave
engine command) documented inline; the defaults reproduce the figures discussed
above.

\par A note on execution time
These studies report no timings on purpose: conditioning is intrinsic to the mesh,
whereas runtime depends on hardware, build flags, and implementation details. As a
rough orientation, mesh generation and the MATLAB/Octave eigen-analysis dominate the
wall-clock cost, while the VEMesh improvement step itself is comparatively
inexpensive; the three-stage pipeline above caches intermediate results so that only
the changed stage is rerun.

