\page ug_polygonal_meshes Polygonal Meshes

[TOC]

### Surface meshes

**Details:** [pmp::SurfaceMesh](https://www.pmp-library.org/classpmp_1_1_surface_mesh.html)

**VEMesh** operates on *planar polygonal meshes* represented as
`pmp::SurfaceMesh` from the
[pmp-library](https://www.pmp-library.org/). The z-coordinate is
ignored.

A few `pmp::SurfaceMesh` methods are particularly useful when getting
started; see the [PMP docs](https://www.pmp-library.org/) for the full
API.

| method | functionality |
| --- | --- |
| `n_faces()` | # faces in the mesh |
| `n_vertices()` | # vertices in the mesh |
| `faces()` | container of faces, use with range-based loops |
| `vertices()` | container of vertices, use with range-based loops |
| `position(pmp::Vertex)` | location of a vertex as a `pmp::Point` |
| `vertices(pmp::Face)` | circulator of vertices of a face |
| `halfedges(pmp::Face)` | circulator of halfedges of a face |
| `face(pmp::Halfedge)` | face to which a halfedge belongs |
| `is_boundary(pmp::Vertex)` | does a vertex lie on the boundary |
| `is_boundary(pmp::Halfedge)` | does a halfedge belong to the boundary |
| `is_boundary(pmp::Face)` | does a face have any edge along the boundary |

VEMesh depends on pmp only for the mesh data structures, not for its
algorithms.


### Vertex ordering convention

VEMesh assumes that all polygonal faces are oriented consistently, with
vertices listed in **counter-clockwise (CCW) order** in the (x,y) plane.
This convention is required by the underlying quality metrics — `geom_shape`
relies on signed area, and the VEM stiffness matrix construction in
`vem_stability_ratio` uses CCW-oriented edge normals.

The mesh readers `vm::read_off` and `vm::read_vtk` expect this
convention.  In debug builds, internal asserts catch CCW violations at
critical locations. In release builds, behavior is undefined for
CW-oriented input.

### Face/vertex properties

`pmp::SurfaceMesh` enables attaching typed data to vertices and faces.
VEMesh uses this to store two required labels:

- **`domain_id`** — an integer label per face, distinguishing
  subdomains. Useful when the domain has embedded interfaces.
- **`interface_id`** — an integer label per vertex, marking those
lying on boundaries or embedded interfaces.

Every VEMesh mesh must have both properties. Add them with:

```cpp
pmp::SurfaceMesh mesh;
...
auto domain_ids    = mesh.add_face_property<int>("domain_id");
auto interface_ids = mesh.add_vertex_property<int>("interface_id");
```

Access and assign them with:

```cpp
auto domain_ids    = mesh.get_face_property<int>("domain_id");
auto interface_ids = mesh.get_vertex_property<int>("interface_id");
...
domain_ids[f]    = 12;   // f is of type pmp::Face
interface_ids[v] = 2;    // v is of type pmp::Vertex
```

- **Convention:** `domain_id = 0` denotes a default, single-domain mesh.
- **Contract:** `interface_id = -1` identifies a vertex as
  **unconstrained** — eligible for relaxation.
- **Contract:** Boundary vertices are always constrained, regardless
of `interface_id`.

Face and vertex qualities can also be stored as properties. The VTK
writer reads them under default tags:

- face qualities under `vm::Face_Quality_Tag = "face_quality"`
- vertex qualities under `vm::Vertex_Quality_Tag = "vertex_quality"`

When present, these fields are written to the output. Custom tags work
too, but the default writers will skip them. This is useful for
comparing qualities before and after updates, or across different
metrics. See \ref ug_io "Mesh I/O" for the read/write behaviour of
each supported format.

Add face/vertex quality properties with:

```cpp
pmp::SurfaceMesh mesh;
...
auto face_qualities   = mesh.add_face_property<double>("face_quality_stability_ratio", 0);
auto vertex_qualities = mesh.add_vertex_property<double>("vertex_quality_shape_ratio", 0);
```

The trailing `0` is the default value. Access and modify with:

```cpp
auto face_quality_values   = mesh.get_face_property<double>("face_quality_stability_ratio");
auto vertex_quality_values = mesh.get_vertex_property<double>("vertex_quality_shape_ratio");
...
face_quality_values[f]   = 0.23;   // f is of type pmp::Face
vertex_quality_values[v] = 0.45;   // v is of type pmp::Vertex
```



