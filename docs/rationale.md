# Rationale for VEMesh {#rationale}

### Guiding principle

VEMesh is built on the premise that robust mesh improvement can be
achieved through simple, local, and provably beneficial operations.
Rather than relying on global optimization or heuristic repair
strategies, the library applies small, quality-driven updates—element
agglomeration and vertex relaxation—that are accepted only when they
strictly improve a user-defined quality metric.

All operations are **local, atomic, and metric-aware**: each update
acts on a single face or vertex, preserves mesh validity and topology,
and is evaluated independently using the supplied quality evaluator.
Poor-quality entities are prioritized through queue-based traversal,
ensuring that computational effort is focused where it is most
effective.

VEMesh emphasizes **extensibility and control**. By remaining agnostic
to the choice of quality metric and exposing fine-grained control over
update ordering and acceptance criteria, VEMesh enables users to tailor
mesh improvement strategies to specific numerical methods—most notably
the Virtual Element Method—without embedding method-specific
assumptions into the library.

### Paucity of tools

Surface discretizations in computer graphics and numerical methods are
dominated by triangle and quadrilateral meshes—and for good reason.
Robust mesh generation tools make such meshes easy to generate and
adapt, while geometric algorithms and approximation theories built on
them are well developed.

The same cannot be said for general polygonal meshes. With notable
exceptions, comparatively few algorithms and tools exist for working
with polygonal discretizations. In this context, VEMesh serves as a
tool for polygonal mesh improvement.

### Context for polygonal meshes

The benefits of directly handling polygonal meshes are becoming
increasingly apparent in specific contexts. For instance:
- Embedding boundaries and interfaces in non-conforming meshes
  naturally leads to polygonal elements, as commonly encountered in
  moving boundary problems.
- Merging overlapping tessellated surfaces—a common requirement for
  constructing watertight geometries—also produces polygonal facets.

It can be argued that local remeshing can eliminate the need to handle
general polygons explicitly. However, even a cursory examination of
the literature suggests that robust and fully automated solutions
remain elusive. At the very least, the current state of the art makes a
compelling case for directly working with polygonal meshes.

### Element improvement in the VEM

The primary motivation for developing VEMesh is to improve the
robustness and effectiveness of the Virtual Element Method (VEM). The
VEM is naturally suited to polygonal meshes, admitting discretizations
that are not permitted by conventional finite element methods and
relieving users from burdensome remeshing and repair procedures—an
especially important advantage in the presence of embedded geometries
and cut cells.

Recent literature has also questioned the conventional wisdom
regarding the significance of element shape. This is particularly
pronounced in the VEM, where geometric regularity is often poorly
correlated with numerical performance. These observations motivate a
departure from traditional mesh improvement techniques that focus
solely on geometric regularity. In this sense, VEMesh enables the
improvement of *elements* rather than merely *faces*.

### Generalization of atomic operations

VEMesh implements two atomic operations for mesh improvement—element
agglomeration and vertex relaxation—both of which can be viewed as
generalizations of familiar operations on triangle and quadrilateral
meshes. For example, agglomeration generalizes the merging of adjacent
triangles into quadrilaterals, while vertex relaxation extends recent
work introducing the notion of vertex quality in triangular meshes.

These two operations serve as primitives for mesh improvement in
VEMesh and can be combined and sequenced flexibly to construct
user-controlled improvement strategies.

### Monotonic improvement

A defining feature of VEMesh is that *every mesh update guarantees
improvement*. A pair of faces is agglomerated, or a vertex is
perturbed, if and only if the operation improves the mesh quality in a
certain sense (see \ref ug_mesh_quality).

This should not be misconstrued as a claim that VEMesh guarantees mesh
improvement—it does not. However, extensive testing suggests that
significant improvement is almost always achieved. Furthermore, VEMesh
deliberately prioritizes poorer-quality faces and vertices, even if
this comes at the expense of degrading better-quality ones.
