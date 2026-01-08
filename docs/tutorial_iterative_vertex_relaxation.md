
\page tutorial_iterative_vertex_relaxation Iterative vertex relaxation

This tutorial demonstrates iteratively relaxing vertices in a
polygonal mesh using vemesh.  
Vertex relaxation perturbing vertices to more favorable locations,
with the goal of improving qualities of elements in the mesh.  
The topology of the mesh is unchanged in the process; just the
locations of vertices are adjusted.  

**Source code:** iterative_vertex_relaxation.cpp

**Overview:**  
- Load a polygonal mesh and choose a face-based quality metric.
- Specify a vertex quality threshold and sampling parameters.
- Initialize a mesh optimizer with the input mesh.
- For a fixed number of iterations:
  - Identify vertices with quality below the prescribed threshold.
  - Relax eligible vertices by sampling candidate locations.
  - Update face and vertex quality measures on the modified mesh.
- Save intermediate meshes and quality vectors to monitor mesh
  improvement.  


[TOC]

## Complete example
\include iterative_vertex_relaxation.cpp

## Explanation

See \ref tutorial_vertex_relaxation for a detailed explanation of
using vemesh for veretx relaxation.  
Specifically:
- loading the mesh using vm::read_off,
- choosing the algorithmic parameters `qepsilon` and `num_samples`,
- choosing the face quality metric (e.g.,  vm::quality::vem_stability_ratio or vm::quality::geom_shape)
- evaluating face/vertex qualities qualities using
  vm::MeshOptimizer::evaluate_face_qualities and vm::MeshOptimizer::evaluate_vertex_qualities,
- instantiating a vm::QualityEvaluator object, 
- saving mesh quality vectors using vm::write_vertex_quality_vector, and
- creating an instance of the vm::MeshOptimizer class
all remain unchanged in this example. For convenience, this example
uses identical variable names as well.

Here, we only discuss details that differ from \ref tutorial_vertex_relaxation.

### Iterative vertex relaxation
```cpp
  const int num_iters = 5; 
  ```
Since the topology of the mesh remains unchanged by vertex relaxation,
iterative mesh improvement by perturbing vertices is especially
appealing. This is unlike with element  agglomeration, which has the
effect of coarsening the mesh.  

While iterative vertex relaxation is generally quite effective, 
mesh quality improvement is not guaranteed.  This can be the case due to:
- the sequential nature of relaxation, i.e., relaxing one vertex after
  another.
- the sampling strategy for finding candidate locations for
  vertices. Choosing a large `num_samples` can help improve meshes
  containing highly skewed polygons or meshes that are already of
  reasonably quality.
  

Relaxation iterations simply requires repeatedly invoking the
relax method:
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
below the given threshold.  We use the 
same threshold `qepsilon` at each iteration to
re-identify vertices requiring improvement. This is not necessary, but is
done here for simplicity. 

In the example, vertex qualities are evaluated at the end of each
iteration and the relaxed mesh is saved as `mesh-iter-0.vtk,
mesh-iter-1.vtk` and so on.  

Notice also that the relax() method is invoked without a
callback. If desired, a callback can be provided, to inspect the mesh
updates within an iteration, or to prematurely terminate an
iteration.  
