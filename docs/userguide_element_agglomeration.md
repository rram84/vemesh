
\page ug_element_agglomeration Element agglomeration  

Element agglomeration is one of the two main operations provided by
VEMesh for mesh improvement. Below, we discuss the rationale for
agglomeration and the algorithm implemented.  

**Details:**  \ref optimizer

[TOC] 

## Rationale

Element agglomeration generalizes the familiar operation of merging
two adjacent triangles into a quadrilateral.

How well agglomeration improves a mesh depends on the choice of \ref
ug_quality_metrics "quality metric". Essentially, the operation merges
pairs of edge-adjacent neighbors when doing so yields a higher
quality. Specifically, let \f$f_1\f$ and \f$f_2\f$ be neighboring
faces sharing a common edge, and \f$f_{12}\f$ the polygon obtained by
merging them. Agglomeration is worthwhile when
\f[ Q(f_{12}) > \min\{Q(f_1),Q(f_2)\}. \f] This is usually the case if
either \f$f_1\f$ or \f$f_2\f$ is poor quality.

## Algorithm

The pseudocode summarises the algorithm. Face qualities are evaluated
through a *quality evaluator* (`QE`) rather than the quality metric
\f$Q\f$ directly — matching the actual implementation.

```text
Input:
  F_cand    : subset of mesh faces eligible for agglomeration
  α         : quality acceptance factor ≥ 1
  QE(·)      : quality evaluator 
  callback  : optional user-defined function

Output:
  nmerged   : number of successful agglomerations

Build priority queue 𝓟 from F_cand,
  ordered by increasing face quality QE(·)

nmerged ← 0

while 𝓟 is not empty do
  f ← pop(𝓟)                 // lowest-quality face

  // pick best among all neighbors for merge
  (found, q_best, nb_best) ← find_best_agglomerable_neighbor(f, QE)

  if found = false then
    continue

  // merge criterion
  if q_best < α · QE(f) then
    continue

  // accept agglomeration
  merge f with nb_best
  nmerged ← nmerged + 1

  if nb_best ∈ 𝓟 then
    remove nb_best from 𝓟

  if callback ≠ null then
    flag ← callback(information + status)

    if flag = false then
      return nmerged

end while

return nmerged
```

The routine `find_best_agglomerable_neighbor` returns the agglomerable
neighbor whose merge with the current face gives the highest resulting
quality. *Agglomerability* is defined in the next section.


## Agglomerability

Merging adjacent faces can isolate vertices (see Fig). While this may
be acceptable in some settings, VEMesh does not permit it; we define
an **agglomerability** criterion to prevent it.

**Criterion 1: isolated vertices**

Let \f$V(f)\f$ denote the set of vertices of a polygon \f$f\f$, and
\f$fg\f$ the polygon obtained by agglomerating \f$f\f$ and \f$g\f$.
Edge-adjacent neighbors \f$f\f$ and \f$g\f$ are *agglomerable* if
\f[V(f\cup g) = V(f) \cup V(g),\f]
which by definition forbids isolated vertices.

**Criterion 2: domain ids**

The second criterion respects embedded interfaces. VEMesh interprets
each face's `domain_id` as a subdomain label. Neighbors \f$f\f$ and
\f$g\f$ are agglomerable only if they share the same `domain_id`,
preventing edges along embedded interfaces from being merged away.

`find_best_agglomerable_neighbor` enforces both criteria when
searching for an optimal neighbor. 

## What to expect

Element agglomeration:
- improves the poorest face qualities with each successful update.
- may degrade better-quality faces along the way — this is by design.
- improves the mesh quality vector \f${\bf Q}_f\f$.
- coarsens the mesh: each successful agglomeration reduces the element
  count by one.
- may not succeed even when a face requires improvement — its
  agglomerable neighbors may not yield enough quality gain.
- alters the mesh topology.
- leaves vertex count and positions unchanged, preserving VEM
  degrees of freedom.
- preserves domain interfaces.
- creates no isolated vertices; every vertex remains incident on at
  least one face, so updated meshes remain usable with first-order VEM.
- preserves mesh validity — no degenerate or overlapping elements
are produced.

## Usage

`vm::MeshOptimizer` provides three overloaded `agglomerate` methods,
in increasing order of automation:

| Method | Functionality |
|--------|---------------|
| `agglomerate(const pmp::Face&, const QualityEvaluator&, double)` | Attempts to merge a single given face with an agglomerable neighbor. |
| `agglomerate(const std::set<pmp::Face>&, const QualityEvaluator&, double, const ProgressCallback&)` | Attempts agglomeration on a user-supplied subset of faces, poorest first. |
| `agglomerate(const QualityEvaluator&, double, double, const
|ProgressCallback&)` | Performs a mesh-wide search for faces with
|quality below a threshold \f$\epsilon\f$, then attempts agglomeration
|on them, poorest first. |
