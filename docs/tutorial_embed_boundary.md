

\page tutorial_embed_boundary Improving a mesh resulting from embedding a boundary   

This tutorial demonstrates:
- embedding a circular boundary in a non-conforming triangle mesh 
- identifying subsets of faces and vertices in the vicinity of the boundary that require improvement
- using vemesh to iteratively and alternately
	- relax just the identified set of vertices
    - using vemesh to agglomerate just the identified set of faces

**Source code:** embed_boundary.cpp

**Overview:**  
- Define a signed distance level set for the boundary.
- Perturb background mesh nodes to ensure no node lies arbitrarily close to the zero level set.
- Embed the boundary by clipping intersected elements and tagging boundary vertices (`interface_id`).
- Iteratively improve the mesh near the boundary:
  - relax low-quality interior vertices adjacent to the boundary,
  - agglomerate low-quality faces incident on the boundary.
- Output the improved embedded mesh; faces/vertices away from the boundary
  remain unchanged.
  
[TOC]

## Complete example
\include embed_boundary.cpp

## Explanation

See \ref tutorial_iterative_element_agglomeration and 
\ref tutorial_iterative_vertex_relaxation for explanations of
using vemesh for iterative element agglomeration and vertex
relaxation.  

Here, we focus on:  
- using \ref utilities provided as part of these tutorial examples to
embed a boundary in a non-conforming mesh using a level set function.  
- identifying candidate faces for agglomeration and candidate vertices
for relaxation
- iteratively relaxing the identified vertices and agglomerating the
  identified faces using overloaded methods in the vm::MeshOptimizer class.

### Embedding a boundary in a non-conforming mesh
**Mesh and level set function:**  
```cpp  
  // input triangle mesh over [-1,1] x [-1,1] 
  const std::string meshfile = "sample_data/tri/bbbb-3.off";
  pmp::SurfaceMesh tri_mesh = vm::read_off(meshfile);
  
  // center and radius of circular domain
  const double circ_center[] = {0.,0.};
  const double circ_radius = 1./std::sqrt(3.);
  const int domain_id = 12;

  // level set function for circular boundary
  vm::tutorial::LevelSetFn ls_circ =
    [circ_center, circ_radius](const double* X) {
    double Y[] = {X[0]-circ_center[0], X[1]-circ_center[1]};
    return std::sqrt(Y[0]*Y[0]+Y[1]*Y[1])-circ_radius;
  };
```  
The example loads a triangle mesh over a square-domain. The faces in
the mesh are all nearly regular and the mesh is of high quality.  

The location of the circular boundary to embed in the mesh is
specified using a level set function, of type
`vm::tutorial::LevelSetFn`, see \ref utilities. In this case, the
level set function is given by \f$\phi({\bf x}) = \|{\bf x}-{\bf
c}\|-R\f$, where the center and radius of the circular boundary are
\f${\bf c}\f$ and \f$R\f$, respectively. The function is negative
within the domain enclosed, zero on the boundary, and positive
elsewhere.  


**Embedding the boundary in the mesh:**  
```cpp
  // perturb mesh nodes away from the zero level set
  const double phi_tol = 1.e-5; 
  const double pert_dist = 10.*phi_tol;
  vm::tutorial::adjust_mesh_nodes(tri_mesh, phi_tol, pert_dist, ls_circ);

  // embed the circular boundary in the perturbed mesh
  pmp::SurfaceMesh embedded_mesh = vm::tutorial::clip_mesh(tri_mesh, phi_tol, ls_circ);
```
The utility functions `vm::tutorial::adjust_mesh_nodes` and
`vm::tutorial::clip_mesh` help embed the circular boundary in the
triangle mesh.  

The embedding algorithm implemented at part of the \ref utilities
avoids *corner* cases in boundary-mesh intersections by assuming that
there is a positive \f$\epsilon>0\f$ such that \f$|\phi|>\epsilon\f$ at all
nodes of the mesh.  

For a given `qepsilon`, conveyed by the parameter `\phi_tol`, the
routine `vm::tutorial::adjust_mesh_nodes` enforces this condition by
perturbing nodes of the mesh, if necessary. The magnitudes of these
perturbations is specified by the parameter `pert_dist`.

In general, `phi_tol` and `pert_dist` should be small fractions of the
 mesh size. Ensuring that `pert_dist` is large compared to `phi_tol`
 improves the chances of enforcing the \f$|\phi|>\epsilon\f$ condition.
 
On successful return from `vm::tutorial::adjust_mesh_nodes(tri_mesh,
  phi_tol, pert_dist, ls_circ)`, it is guaranteed that all nodes of
  `tri_mesh`have \f$|\phi|>\epsilon\f$; a few vertices of the mesh
  lying very close to the boundary may be perturbed in the process.

Then, `vm::tutorial::clip_mesh(tri_mesh, phi_tol, ls_circ)` returns
the result of clipping the triangle mesh with the zero level
set.  
Specifically:  
- the routine computes approximate intersections of the zero level set
  with edges of the mesh by linearly interpolating the level set
  function evaluated at the nodes of `tri_mesh`.
- based on intersections computed this way, the  routine then clips intersected
  triangles and discards the subset deemed to belong to the positive
  sub-level set.
  - clipping triangles of `tri_mesh` this way generates triangles
  and quadrilaterals of `embedded_mesh` along the boundary. Elsewhere, the triangular
  faces of `tri_mesh` are retained unchanged in `embedded_mesh`.
- the intersection points with edges determine nodes of
  `embedded_mesh` that lie on the zero level set. These are assigned
  an `interface_id=1`. The remaining nodes are assigned
  `interface_id=-1`.
- all faces of `embedded_mesh` are assigned `domain_id=1`.


### Candidate faces/vertices for improvement  
The faces and vertices of `embedded_mesh` away from the boundary are
retained unchanged from `tri_mesh`, and are hence of excellent
quality. The only candidates faces/vertices of the embedded mesh that
may need improvement are ones in the vicinity of the boundary.  


**Faces for improvement:**  
```cpp
// identify candidate faces for agglomeration
std::set<pmp::Face> identify_candidate_faces(const pmp::SurfaceMesh &mesh,
	                                         const vm::QualityEvaluator &QE,
					                         const double qepsilon)
{
  std::set<pmp::Face> agg_faces{};

  // candidate faces for agglomeration
  // (i) has to have least one node on the zero level set, i.e., interface_id = 1
  // (ii) quality < threshold
  auto interface_ids = mesh.get_vertex_property<int>("interface_id");
  auto faces = mesh.faces();
  for(auto f:faces)
    {
      auto vertices = mesh.vertices(f);
      for(auto v:vertices)
	if(interface_ids[v]==1)
	  {
	    
	    double qval = QE(f, mesh); // quality of this face
	    if(qval<qepsilon)
	    // this is a candidate face for agglomeration
	    agg_faces.insert(f);
	    break;
	  }
    }

  return agg_faces;
}
```
We expect that only faces of `embedded_mesh` with one or more vertices
on the boundary can be of poor quality. Hence, we identify such faces
and evaluate their quality. If their quality lies below a specified
threshold, we tag the face as a candidate for agglomeration and insert
them into the container `agg_faces`.

**Vertices for improvement:**  
```cpp
// identify candidate vertices for relaxation
std::set<pmp::Vertex> identify_candidate_vertices(const pmp::SurfaceMesh &mesh,
						                          const vm::QualityEvaluator &QE,
						                          const double qepsilon)
{
  std::set<pmp::Vertex> relax_vertices{};
  auto interface_ids = mesh.get_vertex_property<int>("interface_id");
  
  // (i)   should not lie on the zero level set
  // (ii)  should be connected to a node on the zero level set by an edge in the mesh
  // (iii) quality < threshold
  auto vertices = mesh.vertices();
  for(auto v:vertices)
    if(interface_ids[v]==1) // this is a boundary node
      {
	auto vertex_nbs = mesh.vertices(v); // its 1-ring
	for(auto w:vertex_nbs)
	  if(interface_ids[w]==-1)          // this is an interior node
	    {
	      double qval = QE(w, mesh);    // quality of this vertex
	      if(qval<qepsilon)
		relax_vertices.insert(w);   // this is a candidate vertex for relaxation
	    }
      }
  
  return relax_vertices;
}
```

In a similar vein, vertices of `embedded_mesh` away from the boundary
that are retained undisturbed from `tri_mesh` have excellent
quality. It suffices to inspect vertices near the boudnary.

Specifically:  
- Nodes of `embedded_mesh` lying on the boundary cannot be
  perturbed. This is enforced by vm::MeshOptimizer, which only permits
  relaxing nodes with `interface_id=-1`.
- Interior nodes of `embedded_mesh` belonging to the one-ring of the
  boundary nodes are candidates for relaxation. This is because these
  nodes lie on edges clipped by the boundary.
- Among these, just a subset of vertices of poor quality need to be
  relaxed.   
These are precisely the vertices identified in the set
`relax_vertices`.   
  
Note that the quality test is performed on interior vertices, even
though the search is driven by boundary vertices. This reflects the
fact that boundary nodes are fixed, and only interior nodes can be
perturbed.  


### Relaxation and agglomeration near the boundary    
```cpp
  // --- iteratively optimizer ---
  for(int iter=0; iter<num_iters; ++iter) {
    
    std::cout << "\n\n Iteration " << iter <<": " << std::flush;

    // relax 
    auto relax_vertices = identify_candidate_vertices(mesh, QE, qepsilon); // candidate vertices
    int num_relaxed = optimizer.relax(relax_vertices, QE, num_samples);
    std::cout << "\nrealxed " << num_relaxed << " vertices " << std::flush;

    // agglomerate
    auto agg_faces = identify_candidate_faces(mesh, QE, qepsilon); // candidate faces
    int num_agg = optimizer.agglomerate(agg_faces, QE, qfactor);
    std::cout << "\nagglomerated " << num_agg << " faces " << std::flush;
    ...
```
We then proceed to iteratively improving the mesh by iteratively
relaxing vertices and agglomerating faces.  
For simplicity:  
- at each iteration, we (re)identify candidate vertices for relaxation
- we relax the identified vertices.   
- (re)identify candidate faces for agglomeration
- subsequently, we agglomerate faces.  

Notice that:
- we re-identify candidate vertices/faces at each iteration because
the mesh is revised through relaxation and agglomeration
operations. The spirit of the mesh improvement iterations is to make
as few modifications to the mesh as possible to improve its quality beyond a specified threshold.  
- we use the same quality metric for both vertex relaxation and
  element agglomeration. This is not necessary, but is a reasonable
  choice.  
- the order of operations- relaxation followed by agglomeration, is
  chosen for definiteness. Other combinations and sequences of
  operations are possible as well, and can be easily implemented
  through a minor modification of the code snippet shown.  
- the relax() and agglomerate() do not take the quality threshold
 paramater `qepsilon`. This is because the subset of vertices/faces to
 be examined for relaxation/agglomeration are provided directly. In
 particular, these methods do no independently identify vertices/faces
 to improve.  

Monitoring the number of relaxed and agglomerated faces will help
judge the need of iterations of improvement required.
