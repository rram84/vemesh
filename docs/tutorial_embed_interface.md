
\page tutorial_embed_interface Improving a mesh resulting from embedding an interface  

This tutorial demonstrates:
- embedding an interface represented as a polygonal chain in a non-conforming structured quad mesh 
- identifying subsets of faces and vertices in the vicinity of the interface that require improvement
- using vemesh to iteratively and alternately
	- relax just the identified set of vertices
    - using vemesh to agglomerate just the identified set of faces

**Source code:** embed_interface.cpp

**Overview:**  
- Define a signed distance level set from an interface represented as a polygonal chain.
- Generate a structured background mesh of rectangles independently of the interface.
- Perturb background mesh nodes away from the zero level set.
- Embed the interface in the mesh by intersecting edges (approximately) with the zero level set, while tagging vertices (`interface_id`) and faces (`domain_id`) suitably.
- Iteratively improve the embedded mesh near the interface:
  - relax low-quality vertices adjacent to the interface,
  - agglomerate low-quality faces adjacent to the interface, respecting the`domain_id`.
- Output the improved embedded mesh; vertices/faces away from the interface remain unchanged.

[TOC]

## Complete example
\include embed_interface.cpp

## Explanation

See \ref tutorial_embed_boundary for explanation of a closely related example embedding a boundary in a non-conforming mesh.  

Here, we mainly focus on using \ref tutorial_utils "tutorial utilities" provided as part of these tutorial examples to define and
embed an interface in a non-conforming mesh using a level set function.  

### Embedding an interface in a non-conforming mesh
**The interface:**  
The interface is given by an ordered sequence of sample points read from a file, one `x y` pair per line. These points are the vertices of a simple polygon whose boundary is the interface.  

**The level set:**  
The location of the interface is specified using a level set function of type `vm::tutorial::LevelSetFn`, see \ref tutorial_utils "tutorial utilities". Here it is provided by `vm::tutorial::PolygonSDF`, the signed distance to the interface polygon:  
```cpp
  // signed distance to the interface polygon (read from the sample file)
  const vm::tutorial::PolygonSDF interface_sdf(filename_interface_vertices);

  vm::tutorial::LevelSetFn sdfunc =
    [&interface_sdf](const double* X) { return interface_sdf(X); };
```  
`vm::tutorial::PolygonSDF` evaluates the signed distance to the polygon boundary: its magnitude is the Euclidean distance to the nearest boundary segment, and its sign is determined by whether the evaluation point lies inside the polygon. Adopting the convention that the negative sub-level set coincides with the polygon enclosed by the interface, the level set vanishes on the interface, is negative at points inside the polygon, and is positive elsewhere. The boundary segments are stored internally in an R-tree, so each evaluation stays fast even for finely sampled interfaces; see \ref tutorial_utils.

**Background mesh:**   
A structured rectangle mesh over a square domain  is generated using the utility `vm::tutorial::create_rectangle_mesh`.  
```cpp
  const std::array<double,2> left_cnr{0.,0.}; // bottom left corner
  const int nx = 15; // #nodes along x
  const int ny = 15; // #nodes along y
  const double hx = 1./static_cast<double>(nx-1); // grid size along x
  const double hy = 1./static_cast<double>(ny-1); // grid size along y
  auto rect_mesh = vm::tutorial::create_rectangle_mesh(left_cnr, hx, nx, hy, ny);
```

The routine is provided for convenience. The rectangle mesh used can be replaced with any background mesh with refinement commensurate with the interface embedded. 

**Embedding the interface in the mesh:**  
```cpp
  // perturb mesh nodes away from the zero level set
  const double phi_tol = 1.e-5; 
  const double pert_dist = 10.*phi_tol;
  vm::tutorial::adjust_mesh_nodes(rect_mesh, phi_tol, pert_dist, sdfunc);

  // embed the interface in the perturbed mesh
  pmp::SurfaceMesh embedded_mesh = vm::tutorial::embed_interface(rect_mesh, phi_tol, sdfunc);
```  

Embedding an interface in the mesh closely follows the steps discussed in \ref tutorial_embed_boundary. Hence, we first ensure that nodes of the background mesh `rect_mesh` are away from the zero level set by a specified tolerance `phi_tol` by possibly perturbing nodes. The routine `vm::tutorial::adjust_mesh_nodes` achieves this.  

Then, the utility `vm::tutorial::embed_interface` embeds the interface in `rect_mesh`.  Specifically:  
- the routine computes approximate intersections of the zero level set
  with edges of the mesh by linearly interpolating the level set
  function evaluated at the nodes of `rect_mesh`.
- based on intersections computed this way, the  routine then partitions each intersected
  face into polygons that lies in the positive and negative sub-level sets.
  - clipping rectangles (or quadrilaterals in general) of `rect_mesh` this way generates triangles, 
  quadrilaterals, pentagons and hexagons, all included in the `embedded_mesh`.
  - faces of `rect_mesh` away from the interface are retained unchanged in `embedded_mesh`.
- the intersection points with edges determine nodes of
  `embedded_mesh` that lie on the zero level set. These are assigned
  an `interface_id=1`. The remaining nodes are assigned
  `interface_id=-1`.  
- all faces of `embedded_mesh` belonging to the negative sub-level set are assigned `domain_id=1`, while those belonging to the positive sub-level set are assigned `domain_id=2`. This helps distinguish faces on either side of the interface.



### Relaxation and agglomeration near the interface   
```cpp
for(int iter=1; iter<=num_iters; ++iter) {
    
    std::cout << "\n\n Iteration " << iter <<": " << std::flush;

    // relax 
    auto relax_vertices = identify_candidate_vertices(mesh, QE, qepsilon); // candidate vertices
    int num_relaxed = optimizer.relax(relax_vertices, QE, num_samples);
    std::cout << "\nrelaxed " << num_relaxed << " vertices " << std::flush;

    // agglomerate
    auto agg_faces = identify_candidate_faces(mesh, QE, qepsilon); // candidate faces
    int num_agg = optimizer.agglomerate(agg_faces, QE, qfactor);
    std::cout << "\nagglomerated " << num_agg << " faces " << std::flush;

    // evaluate mesh qualities and save file
    optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
    vm::write_vtk(mesh, outdir+"/mesh-iter-"+std::to_string(iter)+".vtk");
    vm::write_face_quality_vector(mesh, outdir+"/qvec-iter-"+std::to_string(iter)+".dat");
  }
  ```  
  
Mesh improvement in `embedded_mesh` is limited to vertices and faces in the vicinity of the interface.   
Just as done in \ref tutorial_embed_boundary, we follow an iterative relax-then-agglomerate sequence.  

At each iteration, we (re)identify vertices in `embedded_mesh` adjacent to the interface and on either side of it, that are of poor quality. These are included in the subset `relax_vertices`.  

Similarly, the subset of faces `agg_faces` consists of faces with at least one vertex on the interface. These are considered for agglomeration. It is important to note that the agglomerate method automatically respects the interface by permitting agglomeration only of faces with identical `domain_id`.  
	
Candidate vertices and faces are reidentified at every iteration to reflect changes in mesh topology and quality.

