
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

Here, we mainly focus on using \ref utilities provided as part of these tutorial examples to define and
embed an interface in a non-conforming mesh using a level set function.  

### Embedding an interface in a non-conforming mesh
**The interface:**  
Given an ordered sequence of sample points, the interface is defined as a line string joining them in the same order.  

```cpp
void get_interface(const std::string filename,
                   vm::boost_polygon_t &polygon,
		           vm::boost_linestring_t &linestring)
{
  // read the polygon vertices
  std::ifstream file(filename);
  if(!file.is_open())
    throw std::runtime_error("Could not open file to read interface nodes "+filename);

  // create polygon
  polygon.clear();
  double x, y;
  while(file >> x >> y)
    vm::bg::append(polygon.outer(), vm::boost_point_t(x, y));
  file.close();
  vm::bg::correct(polygon);
  
  // line string representation of the interface
  linestring.clear();
  for(auto& it:polygon.outer())
    vm::bg::append(linestring, it);
}

```  

Here, the sample points provided define the line string representation, as well as the vertices of a polygon.  
As we will see next, the polygon is used to determine the sign of the level set function, while the line string is used to compute its magnitude.  



The location of the interface is specified using a level set function, of type `vm::tutorial::LevelSetFn`, see \ref utilities. In this case, the level set function is defined by the lamba:  
```cpp
  vm::tutorial::LevelSetFn ls_interface =
    [&interface_linestring, &interface_polygon](const double* X) {
    bool is_inside = vm::bg::within(vm::boost_point_t(X[0],X[1]), interface_polygon);
    double dist = vm::bg::distance(vm::boost_point_t(X[0],X[1]), interface_linestring);
    return (is_inside==true) ? -dist : dist;
  };
```  
The magnitude of the level set function is taken as the Euclidean distance to the line string representation of the interface.
 Adopting the convention that the negative sub-level set coincides with the polygon enclosed by the interface, the sign of the level set function is determined by checking if the point of evaluation lies within the polygon.   

The level set function defined this way vanishes at the interface, is negative at points belonging to the polygon enclosed by the interface, and is positive elsewhere.

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
  vm::tutorial::adjust_mesh_nodes(rect_mesh, phi_tol, pert_dist, ls_interface);

  // embed the interface in the perturbed mesh
  pmp::SurfaceMesh embedded_mesh = vm::tutorial::clip_mesh(rect_mesh, phi_tol, ls_interface);
```  

Embedding an interface in the mesh closely follows the steps discussed in \ref tutorial_embed_interfaces. Hence, we first ensure that nodes of the background mesh `rect_mesh` are away from the zero level set by a specified tolerance `phi_tol` by possibly perturbing nodes. The routine `vm::tutorial::adjust_mesh_nodes` achieves this.  

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

