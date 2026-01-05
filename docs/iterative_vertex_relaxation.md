
\page iterative_vertex_relaxation Iterative vertex relaxation

This tutorial demonstrates how to iteratively relax vertices in a polygonal mesh using vemesh.

**Source code:** iterative_element_agglomeration.cpp

It starts by loading a mesh with poor-quality elements, the goal is to improve
element quality by face agglomeration driven by a user-defined quality metric.

**Load the mesh:**
```cpp
const std::string meshfile = "sample_data/sorgente/mesh3_20.off";
... 
pmp::SurfaceMesh in_mesh = vm::read_off(meshfile);

```
vemesh provides readers for OFF and VTK file formats. See @ref io
for details.

**Algorithmic options:**
```cpp
  const int num_iters = 5;     
  const double qepsilon = 0.2; 
  const double qfactor = 1.2;  
  ```
The parameter `qepsilon`, in the range [0,1], defines the threshold
for identifying elements of poor quality. The agglomerate method of
the vm::MeshOptimizer class used in this example first tags faces
whose qualities fall below this threshold and improves them in
ascending order of quality.  

The parameter `qfactor`, assumed to be larger than 1, dictates whether
an agglomerated face is acceptable or not. If a face \f$f\f$ can be
agglomerated to a new face \f$g\f$, then the result is accepted only
of \f$Q(g)\geq {\rm qfactor}\times Q(f)\f$. The parameter mainly helps
prevent over-agglomeration, and avoids faces with large number of
vertices/edges.

Iterative agglomeration can substantially improve element quality. The
number of iterations to execute is specified by the parameter
`num_iters`, which is assumed to be positive.  In this example, the
same quality threshold `qepsilon` is used after each iteration to
re-identify faces requiring improvement.

**Face quality:**   
```cpp
	const auto face_quality_metric = vm::quality::geom_min_angle; 
    vm::QualityEvaluator QE(face_quality_metric);
```
vemesh provides users complete freedom in defining
the metric used to evaluate face qualities, see @ref quality.  

Three specific examples are provided for users to test out:  
- vm::quality::vem_stability_ratio: uses the ratio of extreme eigenvalues of
  the element-level stiffness matrix in the VEM to define element
  quality in a shape-agnostic manner  
- vm::quality::geom_shape: defines a geometric measure of polygon quality. It uses a non-dimensional ratio of the area and the
  squared-perimeter, so that more regular polygons are assigned a
  better quality closer to 1.
- vm::quality::geom_min_angle: defines a geometric measure of polygon
  quality. It measures the quality of a n-sided polygon as the tatio
  of its smallest included angle to the included angle in a regular
  n-sided polygon. 
  
This example uses the element stability ratio as the quality metric.

The vm::QualityEvaluator class provides different interfaces to
evaluate face/vertex qualities.

**Mesh optimizer:**  
```cpp
	vm::MeshOptimizer optimizer(in_mesh);
   auto& mesh = optimizer.get_mesh();
```	
The class vm::MeshOptimizer implements element
agglomeration and vertex relaxation functionalities in the library. 

It takes an instance of the mesh to be improved at construction and
make a copy. All operations on the mesh executed by the optimizer are
peformed on this copy, which can be accessed immutably using the
`vm::MeshOptimizer::get_mesh()` method.

**Face/vertex quality evaluation:**  
```cpp
   optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
   optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
   vm::write_vtk(mesh, outdir+"/input_mesh.vtk");
   vm::write_face_quality_vector(mesh, outdir+"/qvec-input.dat");
```
The vm::MeshOptimizer class evaluates qualities of faces using the
vm::MeshOptimizer::evaluate_face_qualities method. The evaluated
qualities are stored as a face property in the mesh using the tag
provided. In this example, we use the *default* tag
'vm::Face_Quality_Tag = "face_quality"`. Access the evaluated face qualities as:
```cpp
auto face_qualities = mesh.get_face_property<double>(vm::Face_Quality_Tag);
```

By virtue of their definition (see @userguide), vertex qualities are
evaluated using face qualities. Hence, the
`evaluate_vertex_qualities()` method takes the property tag storing
face qualities in the mesh as input. In this example, the evaluated
qualities are saved under the *default tag*
`vm::Vertex_Quality_Tag="vertex_quality"`. 
Access the evaluated vertex qualities as:
```cpp
auto face_qualities = mesh.get_face_property<double>(vm::Face_Quality_Tag);
```

Notice that it is possible to evaluate and store multiple face quality
metrics, and corresponding vertex quality metrics in a mesh. However,
mesh @io functions only look for the default tags given by
`vm::Face_Quality_Tag` and `vm::Face_Quality_Tag` when saving meshes
to files.


**Iterative agglomeration:**  
```cpp  

   for(int iter=0; iter<num_iters; ++iter) {
      ... 
     int num_agg = optimizer.agglomerate(QE, qepsilon, qfactor);
      ...
     optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
     optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
     vm::write_vtk(mesh, outdir+"/mesh-iter-"+std::to_string(iter)+".vtk");
     vm::write_face_quality_vector(mesh, outdir+"/qvec-iter-"+std::to_string(iter)+".dat");
  }
```
At each iteration, the optimizer identifies faces whose qualities fall
below the given threshold `qepsilon`. Internally, the agglomerate()
method maintains a priorty queue of faces ordered such that the poorest face appears at the front
of the queue. The agglomeration algorithm ensures that each face is
agglomerated at most once, and dynamically re-evaluates 
face qualities which may change during the course of agglomeration.

The vm::MeshOptimizer::agglomerate method used here returns the number
of faces successfully agglomerated. It can be used to adaptively
evaluate the need for further agglomeration iterations, for example.

In the example, face qualities are evaluated at the end of each
iteration and the agglomerated mesh is saved as `mesh-iter-0.vtk,
mesh-iter-1.vtk` and so on.



  
**Quality vector:**
```cpp
  vm::write_face_quality_vector(mesh, outdir+"/qvec-input.dat");
  ...
  // agglomeration iterations
  ...
  vm::write_face_quality_vector(mesh, outdir+"/qvec-output.dat");
```
In addition to saving qualities of faces in the mesh, we also save a
*mesh quality vector* that is simply a sorted list of face
qualities. This vector reveals a meaningful and monotonic sense in which the mesh
quality improves after each mesh update, see @userguide.

The first column of the saved file is an index, and the second is the
face quality. The length of the vector is hence the number of faces in
the mesh. In particular, since the input and output meshes have different
number of elements, the lengths of their quality vectors differ as
well.  
Nevertheless,  quality \f$q^{\rm
out}_{n\times 1}\f$ of the output mesh is better
than the quality \f$q^{\rm in}_{m\times 1}\f$ of the input mesh in the
sense that  
\f[ q^{\rm out}_i > q^{\rm in}_i~\text{for each}~i<\arg\min_j q^{\rm out}_j = q^{\rm in}_j.\f]
Visually, the curve representing the output mesh's quality lies
*above* that of the input mesh until their first point of intersection.
