

\page tutorial_custom_quality_metric Specifying a custom quality metric

This tutorial demonstrates implementing a user-defined face quality
metric to perform iterative vertex relaxation to improve a
polygonal mesh using vemesh.  
Specifically, the quality of a polygon is defined as the minimum
interior angle, normalized by the included angle in a regular polygonq.

**Source code:** custom_quality_metric.cpp

**Overview:**  
- Load a polygonal mesh and select parameters for iterative vertex relaxation.
- Define a user-specified face quality metric as a function mapping polygon vertex coordinates to a scalar quality value.
- Instantiate a `vm::QualityEvaluator` using the custom face quality function.
- Use the custom evaluator to compute face and derived vertex qualities.
- Iteratively relax vertices whose qualities fall below a prescribed threshold.
- Update quality measures and save intermediate meshes to assess improvement.

[TOC]

## Complete example
\include custom_quality_metric.cpp

## Explanation

See \ref tutorial_iterative_vertex_relaxation for a detailed explanation of
using vemesh for iterative vertex relaxation.  
Specifically:
- loading the mesh using vm::read_off,
- choosing the algorithmic parameters `qepsilon` and `num_samples`,
- evaluating face/vertex qualities qualities using
  vm::MeshOptimizer::evaluate_face_qualities and vm::MeshOptimizer::evaluate_vertex_qualities,
- instantiating a vm::QualityEvaluator object, 
- saving mesh quality vectors using vm::write_vertex_quality_vector, 
- creating an instance of the vm::MeshOptimizer class, and 
- invoking the relax() method at each iteration  
all remain unchanged in this example. For convenience, this example
uses identical variable names as well.

Here we only discuss details about defining the custom face quality
metric.  

### Face quality metrics  
**vemesh** provides users complete freedom in defining
the metric used to evaluate face qualities.  
These are defined as functions of the type:  
```cpp
  using vm::FaceQualityFn = double(*)(const std::vector<pmp::Point>&);
```
The two quality metrics vm::quality::vem_stability_ratio and
vm::quality::geom_shape are indeed of type vm::FaceQualityFn.

The routine takes a vector of vertices of a polygonal face, ordered in
a counter-clockwise sense. It should return the quality assigned to
the polygon.  

There is considerable literature on reasonable definitions of quality
metrics for polygons, and properties expected of these metrics. In
vemesh, we do not enforce these requirements.   

We only assume that:
- a valid and simple polygon is assigned a positive quality, and
- that polygon qualities can be ordered using the metric, i.e.,
better-quality polygons are assigned larger values.  

It is generally recommended that quality metrics be dimensionless
(e.g., normalized) and lie in the range [0,1]. In practice, this
simplifies the choice of quality thresholds to identify poor quality
faces.

### User-defined quality metric  
In this example, we define the quality of a n-sided polygon as its
minimum interior angle, normalized by the interior angle of a regular
n-sided polygon.  
```cpp
  // custom quality metric =  normalized min included angle of polygon
  double min_angle_metric(const std::vector<pmp::Point>&);
  ...
  // face quality metric
  const vm::FaceQualityFn face_quality_metric = min_angle_metric;
  //  quality evaluator
  vm::QualityEvaluator QE(face_quality_metric);
```  
The routine `min_angle_metric` is used when creating the
`vm::QualityEvaluator` object, and hence in all downstream face and
vertex quality evaluations.

The implementation of the routine is straightforward- loop over the
vertices, evaluate the included angle, and keep track of the minimum:   
```cpp
double min_angle_metric(const std::vector<pmp::Point>& coords)
{
  const int nverts = static_cast<int>(coords.size());
  double min_angle = 2.*M_PI;
  for(int a=0; a<nverts; ++a)
    {
      const auto& Xa = coords[a];
      const auto& Xb = coords[(a+1)%nverts];
      const auto& Xc = coords[(a+2)%nverts];
      
      // angle between edges ab and bc
      const double angle = compute_included_angle_in_degrees(Xa, Xb, Xc);
      
      // track the minimum
      if(angle<min_angle)
	min_angle = angle;
    }

  // normalizing factor
  double norm_angle = M_PI*(nverts-2.0)/nverts;

  // quality measure
  return min_angle/norm_angle;
}
```   
We have used the interior angle in a regular n-sided polygon, equal to
\f$(n-2)\pi/n\f$, to normalize the minimum angle to the range [0,1].  

The helper function `compute_included_angle()` computes the interior
angle in the range \f$[0,2\pi]\f$ at a vertex, when  given the coordinates of the edges incident at it:  
```cpp
double compute_included_angle(const pmp::Point& U, const pmp::Point& V, const pmp::Point& W)
{
  // edges
  const double VU[] = {U[0]-V[0], U[1]-V[1]};
  const double VW[] = {W[0]-V[0], W[1]-V[1]};
  
  // measure the angle at vertex V
  const double dot = VU[0]*VW[0] + VU[1]*VW[1];
  const double det = VU[0]*VW[1] - VU[1]*VW[0];
  double angle     = std::atan2(-det, dot);
  if(angle<0.)
    angle += 2.*M_PI;
  
  return angle;
}
```

### Evaluating multiple qualities   
Although not done in the example, we briefly discuss evaluating
qualities using different quality metrics. For instance, it may be
useful to compare qualities computed using the minimum angle metric
with, say, the shape ratio metric. This can be
done with vemesh as :  
```cpp
  // create quality evaluators for the two metrics to compare
  vm::QualityEvaluator QE_angle(min_angle_metric);
  vm::QualityEvaluator QE_shape(vm::quality::geom_shape);
  
  // evaluate face qualities with the two metrics while associating them with distinct tags
  optimizer.evaluate_face_qualities(QE_angle, "angle_metric");
  optimizer.evaluate_face_qualities(QE_shape, "shape_metric");
```
Then, the qualities assigned to a face can be inspected as:  
```cpp
  // access the evaluated qualities
  auto &mesh = optimizer.get_mesh();
  auto angle_qualities = mesh.get_face_property<double>("angle_metric");  
  auto shape_qualities = mesh.get_face_property<double>("shape_metric");  

  // inspect the quality of a face 'f' in the mesh
  pmp::Face f; 
  ... 
  double angle_quality_of_f = angle_qualities[f];
  double shape_quality_of_f = shape_qualities[f];
```

Of course, vertex qualities can be evaluated and compared in the same
way using the vm::MeshOptimizer::evaluate_vertex_qualities method. 

Note that the @ref io methods for saving meshes to file only look for
the default quality tags `vm::Face_Quality_Tag="face_quality"` and
`vm::Vertex_Quality_Tag="vertex_quality"`. Hence, the qualities saved
in the mesh with user-defined tags will *not* be written to file.
