
\page tutorial_iterative_element_agglomeration Iterative element agglomeration

This tutorial demonstrates iteratively agglomerating low-quality faces
in a polygonal mesh using vemesh to improve element quality.

**Source code:** iterative_element_agglomeration.cpp

**Overview:**  
- Load a polygonal mesh and choose a face-based quality metric.
- Specify a face quality threshold and agglomeration acceptance factor.
- Initialize a mesh optimizer with the input mesh.
- For a fixed number of iterations:
  - Identify faces with quality below the prescribed threshold.
  - Agglomerate each candidate face with a suitable neighbor if the merge improves quality.
  - Update face and vertex quality measures on the modified mesh.
- Save intermediate meshes and face quality vectors to assess mesh
  improvement and coarsening.  

[TOC]

## Complete example
\include iterative_element_agglomeration.cpp

## Explanation

See \ref tutorial_element_agglomeration for a detailed explanation of
using vemesh for element agglomeration.   
Specifically:
- loading the mesh using vm::read_off,
- choosing the algorithmic parameters `qepsilon` and `qfactor`,
- choosing the face quality metric (e.g.,  vm::quality::vem_stability_ratio or vm::quality::geom_shape)
- evaluating face qualities using vm::MeshOptimizer::evaluate_face_qualities,
- instantiating a vm::QualityEvaluator object, 
- saving mesh quality vectors using vm::write_face_quality_vector, and
- creating an instance of the vm::MeshOptimizer class
all remain unchanged in this example. For convenience, this example
uses identical variable names as well.

In the following, we discuss just the details that differ from \ref tutorial_element_agglomeration.

### Agglomeration iterations
```cpp
  const int num_iters = 5;     
  ```
The agglomerate methods of vm::MeshOptimizer class 
agglomerates a poor quality face with a neighbor, resulting a new face
of improved quality.  
But the quality improvement may not be susbtantial/sufficient.  
In such scenarios, iterative agglomeration can be very useful.  The
number of iterations to execute is specified by the parameter
`num_iters`, which is assumed to be positive.   

At the same time, it is detrimental to over-agglomerate because the
mesh becomes progressively coarser (fewer but larger polygons).   

In general, it is helpful to monitor the number of faces successfully
agglomerated after each iteration. This can be used to determine if
further iterations are warranted.  

The agglomeration iterations simply requires repeatedly invoking the
agglomerate method:
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
below the given threshold.   In particular, we use the 
same threshold `qepsilon` at each iteration to
re-identify faces requiring improvement. This is not necessary, but is
done here for simplicity. 

In the example, face qualities are evaluated at the end of each
iteration and the agglomerated mesh is saved as `mesh-iter-0.vtk,
mesh-iter-1.vtk` and so on.  

Notice also that the agglomerate method is invoked without a
callback. If desired, a callback can be provided, to inspect the mesh
updates within an iteration, or to prematurely terminate an
iteration.  
