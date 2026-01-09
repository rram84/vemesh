
# User Guide {#userguide}

We discuss the main concepts and algorithms underlying mesh improvement in vemesh.  
See the \ref @tutorial for examples demonstrating how to use the
functionalities of the library.

[TOC]

## Polygonal meshes  
**See**: [pmp::SurfaceMesh](https://www.pmp-library.org/classpmp_1_1_surface_mesh.html)

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

**Face/vertex properties:** A convenient feature of the mesh class
used is the possibility of storing vertex- and face-based data. We use
this to:  
- assign an integer-valued domain identifier associated with the
  property `domain_id` to faces in the mesh. This is useful when the
  domain has embedded interfaces, for example.  
- assign an integer-valued vertex classifier associated with the
  property `interface_id` to vertices in the mesh. This is useful to
  demarcate vertices lying on boundaries and interfaces in the mesh.  

All meshes in vemesh are expected/required to have the `domain_id` and
`interface_id` property. These can be added easily to an instance
`mesh` as:  
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
   pmp::Face f;
   pmp::Vertex v;
   ...
   domain_ids[f] = 12;
   interface_ids[v] = 2; 
```

See:
pmp::SurfaceMesh polygons, pmp loading meshes domain id, images
interface id face qualities vertex qualities

## Quality metrics
See: 
quality of faces, function signature
vem element stability ratio, weak link with shape
shape ratio
vertex quality
quality evaluator
evaluating qualities

## Element agglomeration
creates polygons
cousin of edge deletion
changes topology
agglomerable neighbors
respecting interfaces
the optimizer class and its overloaded methods
sequential operation
risk of over coarsening, dofs unchanged with vem
mesh validity

## Vertex relaxation
instance of geometric mesh optimization
context in triangle and quad meshes
optimal criterion for mesh. 
sequential, for vertex
min-max problem
ideal scenatio
mesh validity, visibility polygon, sampling strategy


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
