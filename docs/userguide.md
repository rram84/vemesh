
# User Guide {#userguide}

We discuss the main concepts and algorithms underlying mesh improvement in vemesh.  
See the \ref @tutorial for examples demonstrating how to use the
functionalities of the library.

[TOC]

\subpage ug_polygonal_meshes

\subpage ug_quality_metrics

\subpage ug_element_agglomeration

\subpage ug_vertex_relaxation

\subpage ug_utilities

\page ug_polygonal_meshes Polygonal Meshes

[TOC]

### Surface meshes

**vemesh** deals with polygonal meshes. The mesh
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
| `is_boundary(pmp::Vertex)` | does a vertex lie on the boudary |
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
  auto vertex_qualities = mesh.add_face_property<double>("vertex_quality_shape_ratio", 0);  
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
that valid polygons are assigned positive qualities. It is recommened
that quality metrics be normalized to the interval [0,1], which makes
it convenient to defined thresholds to identify faces of poor quality
during mesh improvement iterations.

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

These two routines serves as archetypes of polygon face quality, and
are used in the tests and tutorial examples in the
library. User-defined quality metrics are conveniently accommodated by
simply implementing a routine of type `vm::FaceQualityFn`. An example
can be found in \ref tutorial_custom_quality.


### Vertex qualities  
Based on qualities associated with polygonal faces, vemesh uses a
notion of vertex quality introduced in \ref XX.   
If \f$f_1,\ldots, f_n\f$ are faces indicident at a vertex \f$v\f$,
then, we set:  
\f[ Q(v) = \min\{Q(f_1),\ldots,Q(f_n)\}. \f]
A vertex is hence associated with the poorest quality face incident at
it. The choice of notation here, using \f$Q\f$ for both face and
vertex qualities should not cause confusion- the argument provided to
\f$Q\f$ clarifies this. Moreover, it is useful to retain the same
notation as a reminder that vertex qualites are defined using the face
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
evaluting qualities. For example:  
```cpp
   // quality of a polygon given locations of its vertces
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
*larger*. The ordering relation is meaningful over mesh quality
vectors because it prioritizes the poorest face/vertex qualities.


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

One of the main revalations from recent investigation the VEM is the
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

[TOC] 

## Rationale  
Element agglomerate implemented by vemesh generalizes the well-known
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
agglomeration in vemesh.  

```text
Input:
  F_cand    : subset of mesh faces eligible for agglomeration
  α         : quality acceptance factor
  Q(·)      : quality evaluator 
  callback  : optional user-defined function

Output:
  nmerged   : number of successful agglomerations

Build priority queue 𝓟 from F_cand,
  ordered by increasing face quality Q(·)

nmerged ← 0

while 𝓟 is not empty do
  f ← pop(𝓟)                 // lowest-quality face

  // pick best among all neighbors for merge
  (found, q_best, g_best) ← find_best_agglomerable_neighbor(f, Q)

  if found = false then
    continue

  // merge criterion
  if q_best < α · Q(f) then
    continue

  // accept agglomeration
  merge f with g_best
  nmerged ← nmerged + 1

  if g_best ∈ 𝓟 then
    remove g_best from 𝓟

  if callback ≠ null then
    flag ← callback(information + status)

    if flag = false then
      return nmerged

end while

return nmerged
```

The `find_best_agglomerable_neighbor` routine examines each
neighboring face and determines the quality of the merged result. It
then identifies the best neighbor to agglomerate a face with.  

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
to merge a face with incorporates both restrictions of agglomerability.

## Consequences  

consequence of agglomerating elements is 

creates polygons
cousin of edge deletion
changes topology
agglomerable neighbors
respecting interfaces
the optimizer class and its overloaded methods
sequential operation
risk of over coarsening, dofs unchanged with vem
mesh validity

\page ug_vertex_relaxation Vertex relaxation  


## Vertex relaxation
instance of geometric mesh optimization
context in triangle and quad meshes
optimal criterion for mesh. 
sequential, for vertex
min-max problem
ideal scenatio
mesh validity, visibility polygon, sampling strategy

\page ug_utilities Utilities  



## Some utilities

### I/O
OFF, VTK

### Mesh quality vectors
face and vertex qualities

### Mesh inspection
inspect mesh

### Mesh slicing
clip mesh
embed interface
