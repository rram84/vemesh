
# User Guide {#userguide}

It is a good idea to check out one of the \ref tutorial examples
before getting into the user guide.  

You will see that the examples follow a familiar pattern of:  
- loading a planar polygonal mesh 
- picking/defining a quality metric
- setting up the mesh optimizer
- combining element agglomeration and vertex relaxation operations to
  improve the mesh, and 
- write the result to a file

The user guide follows a similar sequence, while emphasizing the main
concepts and algorithms underlying mesh improvement.  

[TOC]

\subpage ug_polygonal_meshes

\subpage ug_quality_metrics

\subpage ug_element_agglomeration

\subpage ug_vertex_relaxation

\subpage ug_utilities


\page ug_polygonal_meshes Polygonal Meshes

[TOC]

### Surface meshes

**Details:** 
[pmp::SurfaceMesh](https://www.pmp-library.org/classpmp_1_1_surface_mesh.html)  


**vemesh** deals exclusively with *planar polygonal meshes*. The mesh
[pmp::SurfaceMesh](https://www.pmp-library.org/classpmp_1_1_surface_mesh.html)
data structure of the [pmp-library](pmp-library.org) is ideally suited
for this purpose. All meshes in vemesh are hence represented this way,
with the implicit understanding that the z-coordinate is ignored.   

We refer to the documentation pages for details, but note a few
methods of the pmp::SurfaceMesh that will be helpful when getting
started:  

| method | functionality |  
| --- | --- |
|`n_faces()` | # faces in the mesh |
| `n_vertices()` | # vertices in the mesh |
| `faces()` | container of faces, use with range-based  loops |
| `vertices()` | container of vertices in the mesh, use with range-based loops |
| `position(pmp::Vertex)` | location of a vertex as an instance of `pmp::Point` |
| `vertices(pmp::Face)` | circulator of vertices of a face |
| `halfedges(pmp::Face)` | circulator of halfedges of a face |
| `face(pmp::Halfedge)` | face to which a halfedge belongs |
| `is_boundary(pmp::Vertex)` | does a vertex lie on the boundary |
| `is_boundary(pmp::Halfedge)`| does a halfedge belong to the boundary |
| `is_boundary(pmp::Face)`|  does a face have any edge along the boundary |  

vemesh's dependence on the pmp is limited to using the mesh and
related data structures. In particular vemesh does not rely on
algorithms implemented therein.  

### Face/vertex properties  
A convenient feature of the mesh class used is the possibility of
storing vertex- and face-based data. We use
this to:  
- assign an integer-valued domain identifier associated with the
  property `domain_id` to faces in the mesh. This is useful when the
  domain has embedded interfaces, for example.  
- assign an integer-valued vertex classifier associated with the
  property `interface_id` to vertices in the mesh. This is useful to
  demarcate vertices lying on boundaries and interfaces in the mesh.  

All meshes in vemesh are required to have the `domain_id` and
`interface_id` property. These can be added as:  
```cpp
   pmp::SurfaceMesh mesh;
   ...
   auto domain_ids = mesh.add_face_property<int>("domain_id");
   auto interface_ids = mesh.add_vertex_property<int>("interface_id");
```
and accessed/assigned as:  
```cpp
   auto domain_ids = mesh.get_face_property<int>("domain_id");
   auto interface_ids = mesh.get_vertex_property<int>("interface_id");
   ...
   domain_ids[f] = 12;    // f is of type pmp::Face
   interface_ids[v] = 2;  // v is of type pmp::Vertex
```  

By convention:
- `domain_id = 0` denotes a default, single-domain mesh.
- `interface_id = -1` denotes an unconstrained vertex.
- a vertex on the boundary of the mesh is considered constrained,
  irrespective of the `interface_id` assigned to it.

Optionally, face and vertex qualities of a mesh can also be stored as
properties. The utility provided for saving a mesh in vtk format
expects:  
- face qualities under the default tag
  `"vm::Face_Quality_Tag="face_quality"`, and  
-  vertex qualities under the default tag
   `vm::Vertex_Quality_Tag="vertex_quality"`.  

If present, these face-valued and vertex-valued fields are included in
the mesh output.  
Qualities can be associated with other tags as well, with the caveat
that the default mesh write routines will omit them.  
This feature of the mesh data structure can be leveraged to compare
mesh qualities before/after update operations or qualities evaluated
with different metrics.   
Face/vertex qualities can be added an added as:  
```cpp
  pmp::SurfaceMesh mesh;
  ...
  auto face_qualities = mesh.add_face_property<double>("face_quality_stability_ratio", 0);  
  auto vertex_qualities = mesh.add_vertex_property<double>("vertex_quality_shape_ratio", 0);  
```  
which assigns a default value of 0. These properties can be subsequently accessed and modified:  
```cpp
   ...
   auto face_quality_values = mesh.get_face_property<double>("face_quality_stability_ratio");
   auto vertex_quality_values = mesh.get_vertex_property<double>("vertex_quality_shape_ratio);
   ...
   face_quality_values[f] = 0.23;    // f is of type pmp::Face
   vertex_quality_values[v] = 0.45;  // v is of type pmp::Vertex
```


\page ug_quality_metrics Quality metrics  

[TOC]

**Details:** \ref quality

As a mesh improvement tool, vemesh relies on being provided quality
metrics to discriminate between *good/desirable* and *bad/undesirable*
faces. 



### Face qualities  
The library requires a routine that evaluates the quality of a polygonal face:  
```cpp
  using vm::FaceQualityFn =  double(*)(const std::vector<pmp::Point> &coords);
```   
When provided `coords`, the sequence of vertex locations, ordered
counter-clockwise, the oracle returns the quality
associated with the polygon.  
We assume that better quality faces are assigned larger values, and
that valid polygons are assigned positive qualities. It is recommended
that quality metrics be normalized to the interval [0,1]. This
normalization simplifies the choice of thresholds and acceptance
factors used in agglomeration and relaxation algorithms.

The library provides two specific examples of such a routine:  
- `vm::quality::geom_shape`: implements a familiar geometric notion of
quality as a normalized ratio of the area and the squared
perimeter. The normalizing factor is such that a regular polygon is
assigned a maximal quality of 1.  
- `vm::quality::vem_stability_ratio`: determines the quality using the
spectrum of the element stiffness matrix in the virtual element
method. This notion of quality provides a shape-agnostic measure, and
has been demonstrated to be very effective in improving the
conditioning of the stiffness matrix in the VEM.  

These two routines serve as archetypes of polygon face quality, and
are used in the tests and tutorial examples in the
library. User-defined quality metrics are conveniently accommodated by
simply implementing a routine of type `vm::FaceQualityFn`. An example
can be found in \ref tutorial_custom_quality.


### Vertex qualities  
Based on qualities associated with polygonal faces, vemesh uses a
notion of vertex quality introduced in \ref XX.   
If \f$f_1,\ldots, f_n\f$ are faces incident at a vertex \f$v\f$,
then, we set:  
\f[ Q(v) = \min\{Q(f_1),\ldots,Q(f_n)\}. \f]
A vertex is hence associated with the poorest quality face incident at
it. The choice of notation here, using \f$Q\f$ for both face and
vertex qualities should not cause confusion- the argument provided to
\f$Q\f$ clarifies this. Moreover, it is useful to retain the same
notation as a reminder that vertex qualities are defined using the face
quality metric. 

The library does not require/permit defining an independent notion of
vertex quality. These are deduced from the face quality metric
provided.  

### Evaluating face/vertex qualities   
The class vm::QualityEvaluator in the library uses the face quality
oracle to expose convenient interfaces to evaluate qualities of faces
and vertices. An instance of the class is instantiated as:  
```cpp
   vm::QualityEvaluator QE(quality_func); // quality_func is of type vm::FaceQualityFn  
```

Overloaded operators of the class implement different interfaces for
evaluating qualities. For example:  
```cpp
   // quality of a polygon given locations of its vertices
  double q1 = QE(coords);  // coords is of type std::vector<pmp::Point>  
  
  // quality of a face in a mesh
  double q2 = QE(f, mesh); // f of type pmp::Face belongs to mesh of type pmp::Surface  
  
  // quality of a vertex in a mesh
  double q2 = QE(v, mesh); // v of type pmp::Vertex belongs to mesh of type pmp::Surface  
```  

### Mesh quality   
Face qualities can in turn be used to define a notion of
mesh quality. Based on the work of \ref XX, a mesh with faces
\f$f_1,\ldots,f_n\f$ is associated
with the **vector-valued** quality:  
\f[{\bf Q}_f = {\rm asc}(Q(f_1),\ldots,Q(f_n)),\f]  
where \f${\rm asc}\f$ refers to an ascending sort operation. Hence,
the mesh quality vector lists the qualities of faces in order,
starting from the poorest first. 

The vector \f${\bf Q}_f\f$ defines the mesh quality by enumerating face
qualities. Vertex qualities can also be used. If
\f$\{v_1,\ldots,v_m\}\f$ are the vertices of the mesh, then:  
\f[{\bf Q}_v = {\rm asc}(Q(v_1),\ldots,Q(v_n)),\f]  
defines the mesh quality as a sorted list of vertex qualities.  


vemesh does not directly use these vector-valued mesh quality vectors
\f${\bf Q}_f\f$ or \f${\bf Q}_v\f$. Nevertheless, they are useful
because they reveal the sense in which vemesh achieves mesh
improvement. In particular, we note from \ref XX that it is possible
to introduce an ordering relation over vectors
with components in sorted order. Consider a pair of vectors  
\f[ {\bf u}=(u_1\leq u_2\leq \ldots u_n) ~ \text{and}~ {\bf v}=(v_1\leq
v_2\leq \ldots v_n) \f]  
each with components in sorted order.  Then:  
\f[{\bf u}>{\bf v}~\text{if there exists } 1<k\leq n \text { such that
} u_i>v_i~\text{for}~1\leq i<k\f]  
defines an ordering relation.  
When interpreted in the context of mesh quality vectors over the
sequence of mesh updates, the ordering relation lets us examine mesh
improvement. Specifically, an update operation (e.g., a vertex
perturbation) improves the mesh quality if the quality vector is
*larger*. Intuitively, this ordering compares meshes by prioritizing
improvements in their poorest-quality elements/vertices. 


### Remarks  
There is considerable literature on quality metrics for triangles and
quads. These are predominantly geometric in nature. Hence, more
regularly-shaped triangles/quads are assigned better qualities.  

The more general case of qualities of polygonal faces is much less
settled. Natural extensions of the metrics defined for triangles and
quads are often deficient, i.e., do not satisfactorily identify poorly
shaped polygons. Some choice of metrics are also expensive to compute,
rendering them inefficient for practical use. We refer to \ref XX for
related discussions.  

One of the main revelations from recent investigation the VEM is the
poor correlation between polygon shapes and element performance. In
particular, the often touted relationship between element shapes and
performance in the conventional finite element methods appears to be
far more tenuous in the VEM \ref XX. The VEM makes a convincing
argument to resort to element-based quality criteria, rather than
geometric measures of shape. The element stability ratio, implemented
by `vm::quality::vem_stability_ratio` serves precisely this purpose.  


\page ug_element_agglomeration Element agglomeration  

Element agglomeration is one of the two main operations provided by
vemesh for mesh improvement. Below, we discuss the rationale for
agglomeration and the algorithm implemented.  

**Details:**  \ref optimizer

[TOC] 

## Rationale  
Element agglomeration implemented by vemesh generalizes the well-known
operation of merging triangles to create quad faces.  

The purpose of agglomeration as a means of mesh improvement is
intimately tied to the choice of the [ug_quality_metrics](quality
metric). At its essense, the operation attempts to merge pairs of
edge-adjacent neighbors to realize better mesh quality.  Specifically,
let \f$f_1\f$ and \f$f_2\f$ be neighboring faces sharing a common
edge. Let \f$f_{12}\f$ denote the polygonal face resulting from
merging \f$f_1\f$ and \f$f_2\f$. Then, it is possible that:  
\f[ Q(f_{12}) > \min\{Q(f_1),Q(f_2)\}. \f]  
Agglomeration this way is especially meaningful is either of the faces
\f$f_1\f$ or \f$f_2\f$ is deemed to be of *poor* quality.  

## Algorithm  
The pseudocode below concisely summarizes the implementation of
agglomeration in vemesh. Therein, the face qualities are evaluated
using a *quality evaluator* labeled QE, rather than directly using the
quality metric \f$Q\f$. This reflects the implementation more closely.

```text
Input:
  F_cand    : subset of mesh faces eligible for agglomeration
  α         : quality acceptance factor ≥ 1
  QE(·)      : quality evaluator 
  callback  : optional user-defined function

Output:
  nmerged   : number of successful agglomerations

Build priority queue 𝓟 from F_cand,
  ordered by increasing face quality QE(·)

nmerged ← 0

while 𝓟 is not empty do
  f ← pop(𝓟)                 // lowest-quality face

  // pick best among all neighbors for merge
  (found, q_best, nb_best) ← find_best_agglomerable_neighbor(f, QE)

  if found = false then
    continue

  // merge criterion
  if q_best < α · QE(f) then
    continue

  // accept agglomeration
  merge f with nb_best
  nmerged ← nmerged + 1

  if nb_best ∈ 𝓟 then
    remove nb_best from 𝓟

  if callback ≠ null then
    flag ← callback(information + status)

    if flag = false then
      return nmerged

end while

return nmerged
```

The routine `find_best_agglomerable_neighbor` returns the neighboring
face whose agglomeration with the current face yields the highest
resulting face quality, subject to agglomerability constraints. 


## Agglomerability  
An important consideration during agglomeration is that merging
adjacent faces can result in isolated vertices. Fig shows an example
in which this happens. While this may be acceptable, vemesh does
permit this possibility. To this end, we introduce a notion of
**agglomerability** of faces.

**Criterion 1: isolated vertices**  
Let \f$V(f)\f$ denote the set of vertices of a polygon \f$f\f$.  The
result of agglomerating a pair of polygons \f$f\f$ and \f$g\f$ is the
polygon \f$fg\f$, covering the set \f$f\cup g\f$. We say that
edge-adjacent neighbors \f$f\f$ and \f$g\f$ are *agglomerable* if:  
\f[V(f\cup g) =V(f) \cup V(g).\f] The criterion prevents isolated
vertices, essentially by definition. 

**Criterion 2: domain ids**  
A second criterion that restricts agglomerability of neighboring faces
is a consideration of respecting embedded interfaces in the mesh. This
is precisely where the `domain_id` property stored in the mesh is
useful. vemesh interprets the `domain_id` as a label distinguishing
faces belonging to distinct subdomains. Therefore, neighbors \f$f\f$
and \f$g\f$ are considered to be agglomerable only if they have the
same `domain_id`. This prevents an edge of the mesh along an embedded
interface from being deleted by merging the two adjacent faces.

 In the algorithm outlined above, the search for the optimal neighbor
in `find_best_agglomerable_neighbor` to merge a face with incorporates
both restrictions of agglomerability.

## What to expect  

Element agglomeration will:  
- improve the poorest set of element qualities in the mesh with each
  successful update.  
- improve poorer elements often at the expense of better quality
  ones.  
- improve the mesh quality vector \f${\bf Q}_f\f$ of the mesh.   
- coarsen the mesh. Each successful agglomeration will reduce the
  element count by 1.
- not necessarily be successful, even if an element requires
  improvement. This may be because merging an element with its
  agglomerable neighbors may not improve the quality sufficiently.  
- alter the topology of the mesh.  
- leave the vertex count and locations unchanged. This
can be leveraged to preserve the dofs in the VEM.   
- preserve domain interfaces in the mesh.  
- not create isolated vertices, i.e., all vertices in the mesh are
guaranteed to be vertices of polygonal faces in the mesh.  This
feature implies that updated meshes can be used with a first order
VEM.  
- preserve the validity of the mesh, i.e., will not result in
  degenerate/overlapping elements.  

## Usage  

The vm::MeshOptimizer class implements three overloaded methods to
provided different levels of control over agglomeration operations.   

| Method | Functionality |
|--------|---------------|
| `agglomerate(const pmp::Face&, const QualityEvaluator&, double, double)` | Attempts to merge a face with an agglomerable neighbor; provides the most direct control. |
| `agglomerate(const std::set<pmp::Face>&, const QualityEvaluator&, double, double, const ProgressCallback&)` | Attempts agglomerating faces in a specified subset of faces, starting from the poorest one first. |
| `agglomerate(const QualityEvaluator&, double, double, const ProgressCallback&)` | Determines the subset of faces to be considered for agglomeration by performing a mesh-wide search to tag faces with quality below the specified threshold \f$\epsilon\f$. Then attempts agglomerating them, starting from the poorest one first. |

\page ug_vertex_relaxation Vertex relaxation  

[TOC] 

The second functionality provided by vemesh for mesh quality
improvement is vertex relaxation. Specifically, vertices can be
relocated to more favorable positions to improve element qualities in
the mesh. Only unconstrained vertices not lying on the boundary of the
mesh and having `interface_id = -1` are eligible for relaxation.  

**Details:** \ref optimizer

Two main ideas underlie the efficacy of vertex relaxation for mesh
improvement in vemesh:  
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
It is appealing to pose the problem of relocating a vertex, say
\f$v\f$, as an optimization problem:  
\f[\text{Find}~{\bf x} = \arg\max_{{\bf y}\in {\mathbb R}^2}Q(v({\bf
y})),\f]  
where \f$v({\bf y})\f$ denotes the vertex \f$v\f$ when located at
\f${\bf y}\f$. The challenging in resolving this problem stems from
our notion of vertex quality. It transforms the optimization problem
into a the
max-min problem:  
\f[\text{Find}~{\bf x} = \arg\max_{{\bf y}\in {\mathbb
R}^2}\min_{1\leq i\leq n} Q(f_i(v({\bf
y}))),\f]  
where \f$\{f_1,\ldots,f_n\}\f$ denote the faces incident at \f$v\f$
and \f$\{f_1(v({\bf y})),\ldots, f_n(v({\bf y}))\}\f$ denotes their
realizations with vertex \f$v\f$ positioned at \f${\bf y}\f$. The
max-min problem is non-smooth in general, make its resolutions
non-trivial. The choice of the quality metric \f$Q\f$ can render
identifying the optimal location a complicated task, to say the
least.  

In vemesh, we adopt the point of view that it is not essential to
relocate a vertex to an optimal position, since doing so is both an
algorithmically and a computationally expensive proposition in
practice. Instead, we restrict the search for the position of \f$v\f$
to a finite collection of sample points \f${\cal S}(v)\f$. Hence,
vemesh implements vertex relaxation as:  
\f[\text{Find}~{\bf x} = \arg\max_{{\bf y}\in {\cal S}(v)}Q(v({\bf
y})) = \arg\max_{{\bf y}\in {\cal S}(v)}\min_{1\leq i\leq n} Q(f_i(v({\bf
y}))).\f]  
In effect, we no longer find an optimal location for \f$v\f$; we find
a suboptimal one by sampling.   

### Sampling vertex locations  
vemesh implements a two-pronged strategy to generate the set of
candidate locations \f${\cal S}(v)\f$. Let \f$2N\f$ denote a
user-specified count for the number of sample points to inspect.   
- First, we identify vertices \f$\{v_1,\ldots,v_m\}\f$ in the 1-ring
  of \f$v\f$ and can consider their convex combinations:  
\f[ {\bf y} = \left( \sum_{i=1}^m \lambda_i{\bf x}_i\right)\big/
\sum_{i=1}^m \lambda_i,\f] where \f${\bf x}_j\f$ denotes the location
of vertex \f$v_j\f$. The set of weights
\f$\lambda_1,\ldots,\lambda_m\f$ are sampled randomly from a uniform
distribution over \f$(0,1)\f$. We populate the set \f${\cal S}_1(v)\f$
with \f$N\f$ realizations of sample points generated this way.  
- Second, we construct an axis-aligned bounding box \f${\cal B}(v)\f$
  covering the faces incident at \f$v\f$. We generate \f$N\f$ random
  samples contained in \f${\cal B}(v)\f$ to define a second set of
  candidate locations \f${\cal S}_2(v)\f$ for \f$v\f$.  
 
 Fig \ref XX illustrates the two sets of samples generated this
 way. We restrict the search for an improved location of \f$v\f$ to
 the set of \f$2N\f$ sample points contained in \f${\cal S}(v) = {\cal
 S}_1(v)\cup {\cal S}_2(v)\f$.
 
### Feasibility (visibility)  
An important caveat of the sampling strategy adopted to identify
candidate locations is that it does not build in criteria of
feasibility. It is possible, for instance, that relocating \f$v\f$ to
a sample point \f${\bf y}\in {\cal S}(v)\f$ results in a mesh with
overlapping faces. More distressingly, it is not possible to rely on a
user-defined quality metric \f$Q\f$ to rule out this possibility,
which is of a geometric nature. That is, it is possible that
\f$Q(f_i(v({\bf y})))>0\f$ for each face \f$f_i\f$ and yet that
repositioning \f$v\f$ to \f${\bf y}\f$ results in a tangled mesh. This
issue is intimately related to the notion of the [visibility
polygon](https://en.wikipedia.org/wiki/Visibility_polygon). Specifically,
\f$v\f$ can be relocated to \f${\bf y}\f$ only if \f${\bf y}\f$ is
*visible* from each vertex \f$\{v_i\}_{i=1}^m\f$ in its 1-ring. Fig
\ref XX shows an example to this effect, and mesh tangling caused by
repositioning \f$v\f$ outside the visibility polygon of one of the
vertices in its 1-ring.  It is prudent therefore, to ensure that the
set of sample points considered for relocating \f$v\f$ be *visible*
from each of the vertices \f$v_1,\ldots,v_m\f$.

In practice, the visibility polygon of a point is expensive to
[compute](https://doc.cgal.org/latest/Visibility_2/index.html). Our
trials computing visibility polygons of vertices rendered the
relaxation operation extremely slow. Instead, it is more efficient to
explicitly verify feasibility of a location \f${\bf y}\f$ for \f$v\f$
by checking that:  
- the faces \f$\{f_1(v({\bf y})),\ldots,f_n(v({\bf y}))\}\f$ are all
  simple and valid polygons, and  
- pairwise intersections of faces \f$f_i(v({\bf y}))\cap
  f_j(v({\bf y}))\f$ for \f$i\neq j\f$ has zero area measure.  

We term a position satisfying both these conditions as a *feasible*
location for \f$v\f$. Fig XX shows examples of infeasible and feasible
vertex locations. Only feasible locations are considered when
evaluating vertex quality.

In summary, vemesh identifies an improved location for \f$v\f$ from
the \f$2N\f$ samples in \f${\cal S}(v)\f$ as:  
\f[ \text{Find}~{\bf x} = \arg\max_{{\bf y}\in {\cal S}(v)}\min_{1\leq
i\leq n} \{Q(f_i(v({\bf y})))\,:\,\text{such that}~{\bf y}~\text{is
feasible}\}.\f]  
  
  
## Algorithm  

The algorithm implemented for vertex relaxation closely mirrors that
of element agglomeration.  

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

We highlight a few key points.  
- The routine `find_improved_position` implements the sampling
  strategies and feasibility tests.  
- It is possible that a vertex requires improvement but no feasible
  sample is found. This can happen because of the sampling strategy
  adopted does not guarantee feasibility.   
- A vertex is relaxed at most once. Yet, its quality can be revised
  multiple times when vertices in its 1-ring are relaxed.
- The algorithm is only suited for unconstrained vertex
  relaxation. For this reason, vertices lying on the boundary or on
  interfaces cannot be relaxed. In particular, only vertices with
  `interface_id=-1` can be relaxed.  

## What to expect  

Vertex relaxation will:  
- improve the poorest set of vertex qualities, and hence face
  qualities, in the mesh with each  successful update.  
- improve poorer vertices often at the expense of better quality
  ones.  
- improve the  quality vector \f${\bf Q}_v\f$ of the mesh.   
- preverse the element and vertex count.  
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

The vm::MeshOptimizer class implements three overloaded methods to
provided different levels of control over relaxation operations.   

| Method | Functionality |
|--------|---------------|
| `relax(const pmp::Vertex&, const QualityEvaluator&, int)` | Attempts to relax a vertex; provides the most direct control. |
| `relax(const std::set<pmp::Vertex>&, const QualityEvaluator&, int, const ProgressCallback&)` | Attempts relaxing vertices in a specified subset of vertices, starting from the poorest one first. |
| `relax(const QualityEvaluator&, double, int, const ProgressCallback&)` | Determines the subset of vertices to be considered for relaxation by performing a mesh-wide search to tag vertices with quality below the specified threshold \f$\epsilon\f$. Then attempts relaxing them, starting from the poorest one first. |




\page ug_utilities Utilities  

[TOC] 

We briefly discuss a few utilities provided as part of vemesh. These
are not essential functionalities of the library. They are not used in
the main vm::MeshOptimizer class implementing the
agglomeration/relaxation functionalities. Nevertheless, these routines
are invoked in the unit tests and and the tutorial examples.

## I/O  

**Details:** \ref io  

The [pmp-library](https://www.pmp-library.org/group__io.html) provides
utilities for mesh I/O for a few common formats.  vemesh provides a
small set of additional read/write functionalities, specifically for
meshes in OFF and VTK formats:  
| method | functionality | `domain_id`, `interface_id` | `face_quality`, `vertex_quality` |
| --- | --- | --- | --- |  
| vm::read_off | reads vertex coordinates and polygon connectivities from an ascii file in OFF format | not read,  initialized to defaults | not read, not initialized |  
| vm::write_off | writes a mesh to an ascii file in OFF format | not written | not written |  
| vm::read_vtk | reads a mesh from an ascii file in legacy VTK format | read if present; otherwise initialized to defaults | not read, not initialized |  
| vm::write_vtk | writes a mesh to an ascii file in legacy VTK format | written to file | written to file if present |

The VTK format is more informative. It enables incorporating mesh
partitions into subdomains and specifying embedded interfaces in the
mesh through the integer-valued cell-based field `domain_id` and
vertex-based field `interface_id`.  The file format also enables
visualizing face and vertex qualities stored in the default quality
tags `face_quality` and `vertex_quality`.


## Mesh quality vectors  

**Details:** \ref io   

Mesh I/O in VTK format helps visualize face and vertex qualities. To
facilitate a direct inspection of the mesh quality vector, vemesh
provides the routines:  
| routine | functionality |  
| --- | --- |
| `vm::write_face_quality_vector` | writes the mesh quality vector  \f${\bf Q}_f\f$ to an ascii file; 1st column = index, 2nd column =  quality vector component |  
| `vm::write_vertex_quality_vector` | writes the mesh quality vector \f${\bf Q}_v\f$ to an ascii file; 1st column = index, 2nd column = quality vector component |  


## Mesh inspection  

**Details:** \ref utils   

To help check a polygon mesh's validity, vemesh provides the
`vm::inspect_mesh` routine. The routine provides three hierarchical levels of
checks:  
| level | check | expense |   
| --- | --- | --- |  
| vm::MeshInspection::Basic |  vertex and element counts | trivial |   
| vm::MeshInspection::Face | degeneracy of faces | moderate |   
| vm::MeshInspection::Adjacency |  pairwise intersections of faces incident at vertices | expensive |  

In case of errors, the routine returns a vector of codes and messages
as an instance of `vm::MeshInspectionErrors` to help identify
discrepancies.  

The checks performed by `vm::inspect_mesh` represent necessary
conditions on the mesh. They are not sufficient, however.   

None of the mesh improvement methods in vemesh internally invoke these
checks. Nevertheless, it is a good idea to occasionally use these
checks, for instance, when loading a mesh produced by a non-standard
source.  

## Mesh slicing  

**Details:** \ref tutorial_utils

An important use-case of vemesh lies in improving qualities of
polygonal meshes resulting from embedding boundaries and interfaces in
non-conforming meshes. (e.g., structured grids generated by the
utility `vm::tutorial::create_rectangle_mesh`). By way of enabling
users test the efficacy of vemesh in this context, the library
provides two routines:  
| routine | functionality |  
| --- | --- |  
|vm::tutorial::clip_mesh | embed a boundary in a triangle/quad mesh |  
|vm::tutorial::embed_interface | embed an interface in a triangle/quad mesh |  

We record a few common details:  
- Both routines use an instance of `vm::tutorial::LevelSetFn` to
  specify the location of the boundary/interface as the zero level set of a function.  
- Both routines linearly interpolate values of the level set function
to approximately determine edge-boundary and edge-interface intersections.  
- Both routines avoid dealing with corner cases by assuming that
vertices lie away from the zero level set by a user-specified value.
The utility `vm::tutorial::adjust_mesh_nodes` implements minor vertex
perturbations to enforce this condition to achieve a user-defined
tolerance.  
- Neither routine is implemented for efficient performance.  
- Both routines expect a triangle or a quad mesh.  
