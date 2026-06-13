
\page ug_vertex_relaxation Vertex relaxation  

[TOC] 

Vertex relaxation is the second mesh-improvement operation in VEMesh.
It relocates vertices to more favorable positions to improve incident
face qualities. Only **unconstrained** vertices are eligible: vertices
on the mesh boundary or with `interface_id ≠ -1` are skipped.

**Details:** \ref optimizer

Two ideas drive its effectiveness:
- **Vertex quality is the minimum over incident face qualities**
  (\ref ug_quality_metrics). Improving vertex quality therefore
  raises the *worst* face quality at the vertex.
- **A vertex is relocated if and only if its quality improves.** Each
  successful update strictly improves vertex quality, and hence the
  mesh quality vector.
  
The second functionality provided by VEMesh for mesh quality
improvement is vertex relaxation. Specifically, vertices can be
relocated to more favorable positions to improve element qualities in
the mesh. Only unconstrained vertices not lying on the boundary of the
mesh and having `interface_id = -1` are eligible for relaxation.  

**Details:** \ref optimizer

Two main ideas underlie the efficacy of vertex relaxation for mesh
improvement in VEMesh:  
- the notion of vertex quality, as the minimum over the qualities of
  faces incident at a vertex, see \ref ug_quality_metrics. This
  provides a direct relationship between improving vertex qualities
  and improving qualities of faces in the mesh. In particular,
  improving vertex quality necessarily improves qualities of faces
  incident at it.  
- a vertex is relocated if and only if the quality of a vertex is
  improved.  Hence, each vertex update guarantees improvement if
  vertex quality, and consequently, of mesh quality.  

## Vertex updates   

### Optimization perspective

Relocating a vertex \f$v\f$ is naturally posed as an optimization:
\f[\text{Find}~{\bf x} = \arg\max_{{\bf y}\in {\mathbb R}^2}Q(v({\bf y})),\f]
where \f$v({\bf y})\f$ denotes \f$v\f$ when located at \f${\bf y}\f$.

The vertex-quality definition transforms this into a max-min problem:
\f[\text{Find}~{\bf x} = \arg\max_{{\bf y}\in {\mathbb R}^2}\min_{1\leq i\leq n} Q(f_i(v({\bf y}))),\f]
where \f$\{f_1,\ldots,f_n\}\f$ are the faces incident at \f$v\f$ and
\f$f_i(v({\bf y}))\f$ is the realization of \f$f_i\f$ with \f$v\f$
positioned at \f${\bf y}\f$. The problem is non-smooth in general;
depending on \f$Q\f$, identifying the optimum can be expensive.

VEMesh sidesteps the optimal relocation problem. Instead, the search
is restricted to a finite sample set \f${\cal S}(v)\f$:
\f[\text{Find}~{\bf x} = \arg\max_{{\bf y}\in {\cal S}(v)}Q(v({\bf y})) = \arg\max_{{\bf y}\in {\cal S}(v)}\min_{1\leq i\leq n} Q(f_i(v({\bf y}))).\f]
In effect, VEMesh trades optimality for a more tractable, sample-based
search.

### Sampling vertex locations

VEMesh generates sample positions for a vertex in two ways. Let \f$2N\f$ be the
user-specified total number of samples per vertex.

- **Convex combinations of the 1-ring.** Let \f$\{v_1,\ldots,v_m\}\f$
  be the vertices in the 1-ring of \f$v\f$, located at
  \f$\{{\bf x}_1,\ldots,{\bf x}_m\}\f$. We draw samples as
  \f[ {\bf y} = \left( \sum_{i=1}^m \lambda_i{\bf x}_i\right) \big/ \sum_{i=1}^m \lambda_i, \f]
  where the weights \f$\lambda_i\f$ are independent uniform draws on
  \f$(0,1)\f$. This yields \f$N\f$ samples in \f${\cal S}_1(v)\f$.
- **Uniform within the bounding box.** Let \f${\cal B}(v)\f$ be the
  axis-aligned bounding box covering all faces incident at \f$v\f$.
  We draw \f$N\f$ uniform random samples from \f${\cal B}(v)\f$ into
  \f${\cal S}_2(v)\f$.

The search for a position to relocate \f$v\f$ runs over the union of
both sample sets, \f${\cal S}(v) = {\cal S}_1(v) \cup {\cal S}_2(v)\f$.

### Feasibility (visibility)

Sampling does not guarantee feasibility: relocating \f$v\f$ to a
sample \f${\bf y}\in {\cal S}(v)\f$ can produce a tangled mesh with
overlapping faces. Worse, the quality metric \f$Q\f$ cannot rule this
out — even when
\f$Q(f_i(v({\bf y})))>0\f$ for every incident face, the mesh can still
be tangled.

This is the [visibility
polygon](https://en.wikipedia.org/wiki/Visibility_polygon) problem:
\f$v\f$ can be safely relocated to \f${\bf y}\f$ only if \f${\bf y}\f$
is *visible* from each vertex \f$\{v_i\}_{i=1}^m\f$ in its 1-ring;
otherwise the mesh tangles. Sample points must therefore be visible
from every 1-ring vertex.

Computing visibility polygons is
[expensive](https://doc.cgal.org/latest/Visibility_2/index.html).
We instead check feasibility directly:
a location \f${\bf y}\f$ for \f$v\f$ is *feasible* if
- the faces \f$\{f_1(v({\bf y})),\ldots,f_n(v({\bf y}))\}\f$ are
  simple and valid polygons, and
- pairwise face intersections
  \f$f_i(v({\bf y}))\cap f_j(v({\bf y}))\f$ for \f$i\neq j\f$ have
  zero area.

Only feasible samples contribute to the vertex-quality evaluation. The improved location is
\f[ \text{Find}~{\bf x} = \arg\max_{\substack{{\bf y}\in {\cal S}(v)\\{\bf y}~\text{feasible}}}\min_{1\leq i\leq n} Q(f_i(v({\bf y}))). \f]

## Algorithm

The vertex relaxation algorithm mirrors that of element agglomeration.

```text
Input:
  V_cand     : subset of vertices eligible for relaxation
  QE(·)       : vertex quality evaluator, an instance of vm::QualityEvaluator
  N_samples  : number of candidate relocation samples
  callback   : optional user-defined function

Output:
  nrelaxed   : number of successful vertex relaxations

Build priority queue 𝓟 from V_cand,
  ordered by increasing vertex quality QE(·)

nrelaxed ← 0

while 𝓟 is not empty do
  (v, q_old) ← pop(𝓟)        // lowest-quality vertex

  if v is on boundary or on an interface then
    continue

  q_curr ← QE(v)

  // vertex quality may have changed due to neighboring relaxations
  if |q_curr − q_old| > ε then
    insert (v, q_curr) into 𝓟
    continue

  // search for an improved position
  (found, x_new, q_new) ← find_improved_position(v, N_samples, QE)

  if found = false then
    continue

  // accept relocation
  move vertex v to position x_new
  nrelaxed ← nrelaxed + 1

  if callback ≠ null then
    flag ← callback(information + status)

    if flag = false then
      return nrelaxed

end while

return nrelaxed
```

Key points:
- `find_improved_position` implements the sampling strategies and
  feasibility tests.
- A vertex may require improvement and yet not be relaxed — the
  sampling strategy does not guarantee that a feasible improving
  sample will be found.
- A vertex is relaxed at most once, but its quality can be revised
  multiple times when its 1-ring neighbors are relaxed.
- Only unconstrained vertices with `interface_id=-1` can be relaxed
(see the introduction); in particular, vertices lying on the boundary
or on interfaces cannot be relaxed.

## What to expect  

Vertex relaxation will:  
- improve the poorest set of vertex qualities, and hence face
  qualities, in the mesh with each  successful update.  
- improve poorer vertices often at the expense of better quality
  ones.  
- improve the  quality vector \f${\bf Q}_v\f$ of the mesh.   
- preserve the element and vertex count.  
- preserve the topology of the mesh. This can be leveraged to preserve the dofs and matrix data structures in the VEM.     
- not necessarily be successful, even if a vertex requires
  improvement. This is because the sampling-based strategy of
  determining improved vertex locations is not guaranteed to identify
  either a feasible location, or one with better quality.  
- preserve domain interfaces in the mesh.  
- will leave vertices on the boundary of the mesh and along interfaces
  embedded in the mesh undisturbed. 
- preserve the validity of the mesh, i.e., will not result in
  degenerate/overlapping elements.  

## Usage

`vm::MeshOptimizer` provides three overloaded `relax` methods, in
increasing order of automation:

| Method | Functionality |
|--------|---------------|
| `relax(const pmp::Vertex&, const QualityEvaluator&, int, std::optional<unsigned int>)` | Attempts to relax a single given vertex. |
| `relax(const std::set<pmp::Vertex>&, const QualityEvaluator&, int, const ProgressCallback&, std::optional<unsigned int>)` | Attempts relaxation on a user-supplied subset of vertices, poorest first. |
| `relax(const QualityEvaluator&, double, int, const ProgressCallback&, std::optional<unsigned int>)` | Performs a mesh-wide search for vertices with quality below a threshold \f$\epsilon\f$, then attempts relaxation on them, poorest first. |

The trailing `std::optional<unsigned int>` is an RNG seed for
reproducibility/debugging; omit it (or pass `std::nullopt`) otherwise.
