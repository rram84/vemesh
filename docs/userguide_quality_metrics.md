\page ug_quality_metrics Quality metrics  

[TOC]

**Details:** \ref quality

As a mesh improvement tool, VEMesh relies on being provided quality
metrics to discriminate between *good/desirable* and *bad/undesirable*
faces. 



### Face qualities  
The library requires a routine that evaluates the quality of a polygonal face:  
```cpp
    using vm::FaceQualityFn = std::function<double(const std::vector<pmp::Point> &coords)>;
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
can be found in \ref tutorial_custom_quality_metric.


### Vertex qualities  
Based on qualities associated with polygonal faces, VEMesh uses a
notion of vertex quality introduced in \cite rangarajan2017provably.   
If \f$f_1,\ldots, f_n\f$ are faces incident at a vertex \f$v\f$,
then, we set:  
\f[ Q(v) = \min\{Q(f_1),\ldots,Q(f_n)\}. \f]
A vertex is hence associated with the poorest quality face incident
on it. We use the same symbol \f$Q\f$ for both face and vertex
qualities — the argument distinguishes them, and the shared notation
underscores that vertex quality is defined entirely through the face
metric.

The library does not permit defining an independent notion of vertex
quality; it is always derived from the face metric you supply.

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

### Parallel evaluation and thread safety {#ug_quality_parallel}
VEMesh evaluates qualities in parallel using OpenMP, when
available. It does so in the methods
vm::MeshOptimizer::evaluate_face_qualities and
vm::MeshOptimizer::evaluate_vertex_qualities. The parallelization is
especially meaningful in mesh-wide candidate searches
to determine faces that require agglomeration and vertices to be
relaxed. The agglomeration and relaxation methods themselves remain
**sequential**. These operations alter the mesh and the result depends
on the order in which faces/vertices are processed.
Parallelism in VEMesh is therefore limited in scope and **does not
change the result** of an optimization run.

An important point to keep in mind therefore is that a user-defined face
quality oracle (instance of `vm::FaceQualityFn`) can be invoked
**concurrently**, and must therefore be **thread-safe**. The built-in
metrics `vm::quality::geom_shape` and
`vm::quality::vem_stability_ratio` are functions only of input
coordinates and satisfy this requirement.


### Mesh quality {#ug_mesh_quality}  
Face qualities can in turn be used to define a notion of mesh
quality. Based on the work of \cite rangarajan2017provably, a mesh
with faces \f$f_1,\ldots,f_n\f$ is associated
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

VEMesh does not compute these vectors directly; they serve to
*characterise* what mesh improvement means. From
\cite rangarajan2017provably, sorted vectors admit an ordering
relation. Consider a pair of vectors  
\f[ {\bf u}=(u_1\leq u_2\leq \ldots u_n) ~ \text{and}~ {\bf v}=(v_1\leq
v_2\leq \ldots v_n) \f]  
each with components in sorted order.  Then:
\f[{\bf u}>{\bf v}~\text{if there exists a largest } 1<k\leq n \text { such that
} u_i>v_i~\text{for all}~1\leq i<k\f]
defines an ordering relation.  
When interpreted in the context of mesh quality vectors over the
sequence of mesh updates, the ordering relation lets us examine mesh
improvement. Specifically, an update operation (e.g., a vertex
perturbation) improves the mesh quality if the quality vector is
*larger*. Intuitively, this ordering compares meshes by prioritizing
improvements in their poorest-quality elements/vertices. 


### Remarks

Quality metrics for triangles and quadrilaterals are well-studied and
predominantly geometric: more regularly-shaped elements are assigned
better qualities. 

The case of polygon qualities is less settled. Natural extensions of
triangle/quad metrics often fail to flag poorly shaped polygons, and
some are expensive to compute. See \cite sorgente2023survey for a
survey.

Recent investigations of the VEM have established that polygon shape
correlates poorly with element performance — far more tenuously than
in conventional finite element methods \cite
sorgente2022vem,sorgente2022role. This makes the case for
element-based quality criteria over geometric ones;
`vm::quality::vem_stability_ratio` is one such criterion.

