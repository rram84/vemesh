# Rationale for VEMesh {#rationale}

### Guiding principle

VEMesh improves polygonal meshes through small, local, provably
beneficial updates. It applies two primitives- element agglomeration
and vertex relaxation- accepting each only when it strictly improves
a user-defined quality metric. There is no global optimization, no
heuristic repair.

Each update is **atomic**: it acts on a single face or vertex and
preserves mesh validity and topology. Poorer-quality faces and
vertices are prioritised via a queue, so computational effort is
focused where it matters most.

VEMesh is **metric-agnostic**. You supply the quality measure and
control update ordering and acceptance; VEMesh embeds no method-specific
assumptions. This keeps the library useful beyond the VEM, its
original target.



### Element improvement in the VEM

VEMesh was developed for the **Virtual Element Method**. The VEM is
naturally suited to polygonal meshes- it admits discretisations
conventional FEM cannot, and avoids remeshing around embedded
geometries and cut cells.

In the VEM, geometric regularity correlates poorly with numerical
performance. VEMesh therefore improves *elements* rather than merely
*faces*. The user's metric decides acceptance, not face shape.

### Connection to triangle and quad meshes

Both primitives generalise familiar operations on triangle and
quadrilateral meshes. Agglomeration extends the merging of adjacent
triangles into quads. Vertex relaxation extends the notion of vertex
quality from triangular to polygonal meshes.

### Monotonic improvement

Every accepted update strictly improves the mesh quality vector
\f${\bf Q}_f\f$ in the ordering defined in \ref ug_mesh_quality. This is
a genuine guarantee: agglomeration and relaxation are monotone improvers,
and the mesh as a whole improves with every accepted update.

Two caveats are worth knowing. First, the **magnitude** of improvement
is not bounded from below. Hence, an accepted update can yield
arbitrarily small gains, and VEMesh may terminate with a still-poor
mesh if no further updates are accepted. The routines in the library
provides control over this. Second, the guarantee is about the *sorted
quality vector*, not about individual elements: VEMesh deliberately
prioritises the worst faces and vertices and may degrade
better-quality ones along the way.
