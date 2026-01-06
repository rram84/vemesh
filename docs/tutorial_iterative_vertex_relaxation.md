
\page tutorial_iterative_vertex_relaxation Iterative vertex relaxation

This tutorial demonstrates iteratively relaxing vertices in a
polygonal mesh using vemesh.  
Vertex relaxation perturbing vertices to more favorable locations,
with the goal of improving qualities of elements in the mesh.  
The topology of the mesh is unchanged in the process; just the
locations of vertices are adjusted.  

**Source code:** iterative_vertex_relaxation.cpp

[TOC]

## Complete example
\include iterative_vertex_relaxation.cpp

## Explanation

### Load the mesh
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
  const double num_samples = 10;  
  ```
The parameter `qepsilon`, in the range [0,1], defines the threshold
for identifying vertices of poor quality. The relax() method of
the vm::MeshOptimizer class used in this example first tags vertices
whose qualities fall below this threshold and relaxes them to mor
favorable locations in ascending order of quality.  

The parameter `num_samples`, assumed to be larger than 1, dictates the
number of randomly generated candidate sample points that are
inspected to relocate each vertex. In general:  
- only a fraction of the generated sample points are feasible, i.e.,
result in a valid mesh without self-intersections or degenerate faces.
- a larger choice for the number of samples will likely yield more
  improvement. But this comes at the cost of additional computations.

Iterative relaxation can substantially improve vertex quality, and
hence, qualities of faces in the mesh. The
number of iterations to execute is specified by the parameter
`num_iters`, which is assumed to be positive.  In this example, the
same quality threshold `qepsilon` is used after each iteration to
re-identify vertices requiring improvement.

### Vertex quality
```cpp
	const auto face_quality_metric = vm::quality::geom_min_angle; 
    vm::QualityEvaluator QE(face_quality_metric);
```
vemesh provides users complete freedom in defining
the metric used to evaluate face qualities, see @ref quality.  
The vertex quality is defined implicitly using this metric, see @userguide.

Two specific examples are provided for users to test out:  
- vm::quality::vem_stability_ratio: uses the ratio of extreme eigenvalues of
  the element-level stiffness matrix in the VEM to define element
  quality in a shape-agnostic manner  
- vm::quality::geom_shape: defines a geometric measure of polygon quality. It uses a non-dimensional ratio of the area and the
  squared-perimeter, so that more regular polygons are assigned a
  better quality closer to 1.
  
This example uses the geometric measure for the quality metric.

The vm::QualityEvaluator class provides different interfaces to
evaluate face/vertex qualities.

### Mesh optimizer
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

### Face/vertex quality evaluation
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

Since vertex qualities are defined using face qualities, the
`evaluate_vertex_qualities()` method takes the property tag storing
face qualities in the mesh as input. In this example, the evaluated
qualities are saved under the *default tag*
`vm::Vertex_Quality_Tag="vertex_quality"`. 
Access the evaluated vertex qualities as:
```cpp
auto vertex_qualities = mesh.get_vertex_property<double>(vm::Vertex_Quality_Tag);
```

It is possible to evaluate and store multiple face quality
metrics, and corresponding vertex quality metrics in a mesh. However,
mesh @io functions only look for the default tags given by
`vm::Face_Quality_Tag` and `vm::Face_Quality_Tag` when saving meshes
to files.

The face and vertex qualities computed and stored in the mesh this way
are **not** used or accessed during mesh improvement. The agglomerate
and relaxation routines evaluate qualities on the fly, using the
instance of the vm::QualityEvaluator object provided.

### Iterative relaxation
```cpp  

   for(int iter=0; iter<num_iters; ++iter) {
      ... 
     int num_relaxed = optimizer.relax(QE, qepsilon, num_samples);
      ...
     optimizer.evaluate_face_qualities(QE, vm::Face_Quality_Tag);
     optimizer.evaluate_vertex_qualities(vm::Face_Quality_Tag, vm::Vertex_Quality_Tag);
     vm::write_vtk(mesh, outdir+"/mesh-iter-"+std::to_string(iter)+".vtk");
     vm::write_vertex_quality_vector(mesh, outdir+"/qvec-iter-"+std::to_string(iter)+".dat");
  }
```
At each iteration, the optimizer identifies vertices whose qualities fall
below the given threshold `qepsilon`. Internally, the relax()
method maintains a priorty queue of vertices ordered such that the poorest vertex appears at the front
of the queue. The relaxation method ensures that each vertex is
relaxed at most once, and dynamically re-evaluates 
veretx qualities which may change during the course of relaxation.

It is possible that a vertex marked for relaxation may not be
relocated- this can happen for two reasons:  
- the quality of a vertex can improve when a neighboring vertex is
  perturbed. 
- the generated sample points may not improve a vertex's quality,
  which is then left undisturbed. 
  
The vm::MeshOptimizer::relax method used here returns the number of
vertice successfully relaxed. It can be used to adaptively evaluate
the need for further relaxation iterations, for example.

In the example, vertex qualities are evaluated at the end of each
iteration and the relaxed mesh is saved as `mesh-iter-0.vtk,
mesh-iter-1.vtk` and so on.

  
### Quality vector
```cpp
  vm::write_vertex_quality_vector(mesh, outdir+"/qvec-input.dat");
  ...
  // relaxation iterations
  ...
  vm::write_vertex_quality_vector(mesh, outdir+"/qvec-output.dat");
```
In addition to saving qualities of vertices in the mesh, we also save a
*mesh quality vector* that is simply a sorted list of vertex
qualities. This vector reveals a meaningful and monotonic sense in which the mesh
quality improves after each mesh update, see @userguide.

The first column of the saved file is an index, and the second is the
vertex quality. The length of the vector is hence the number of
vertices in the mesh. In particular, since the number of vertices in
the mesh remains unchanged during relaxation iterations, the length of
the quality vector remains a constant.  The quality \f$q^{\rm
out}_{n\times 1}\f$ of the output mesh is better than the quality
\f$q^{\rm in}_{m\times 1}\f$ of the input mesh in the
sense that  
\f[ q^{\rm out}_i > q^{\rm in}_i~\text{for each}~i<\arg\min_j q^{\rm out}_j = q^{\rm in}_j.\f]
Visually, the curve representing the output mesh's quality lies
*above* that of the input mesh until their first point of intersection.
