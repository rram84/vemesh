# vemesh {#aboutvemesh}

*A quality-driven mesh improvement library for planar polygonal meshes*

**Author:**  
**Ramsharan Rangarajan**  
Associate Professor of Mechanical Engineering  
Indian Institute of Science (IISc), Bengaluru, India

---

## Overview  

**vemesh** is a C++ library for improving qualities of planar
polygonal meshes through **quality-driven local updates**.  

The library provides two mesh modification operations— **element
agglomeration** and **vertex relaxation** —both of which are accepted
only when they provably improve a user-supplied quality metric.  Both
operations prioritize poorer elements and vertices to systematically
improves mesh quality while avoiding ad hoc
modifications and expensive non-local optimization.  

Though effective as a general purpose tool for improving polygonal
meshes, the library's name reveals its intended purpose of improving
the robustness and performance of the **Virtual Element
Method (VEM)**.  

## The rationale for vemesh  

Numerical simulations based on the finite element methods are
dominated by triangle and quadrilateral meshes in two spatial
dimensions.  General polygonal meshes, considered in vemesh, are
indeed an unusual choice for scientific computing. 


The library is designed with polygonal discretizations in mind,
particularly, where numerical performance is often weakly correlated
with geometric element shape. Rather than enforcing geometric
regularity, VEMesh allows users to define application-specific quality
metrics and uses them consistently to guide all mesh updates.



planar only
why the name
not a general purpose library
not for generic mesh improvement
exploits vem, relation to vem
requires polygons
generalizes operations of agglomeration to beyond triangles and quads
robust implementations, small overheads
not visualization
not for generic mesh improvement
largely restricted to triangle and quad meshes
new and routine way of realizing polygonal meshes
primary tools for mesh improvement
delaunay and voronoi libraries.
eg collapsing a halfedge only works for triangle meshes

parallelism

sequential nature
