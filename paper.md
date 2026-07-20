---
title: 'VEMesh: Quality-driven polygonal mesh improvement for the virtual element method'
tags:
  - C++
  - virtual element method
  - polygonal meshes
  - mesh quality
  - mesh improvement
  - finite element method
authors:
  - name: Ramsharan Rangarajan
    orcid: 0000-0001-7403-7728
    corresponding: true
    affiliation: 1
  - name: N. Sukumar
    orcid: 0000-0001-6744-7673
    affiliation: 2
affiliations:
  - name: Department of Mechanical Engineering, Indian Institute of Science, Bengaluru, Karnataka 560012, India
    index: 1
  - name: Department of Civil and Environmental Engineering, University of California, Davis, CA 95616, USA
    index: 2
date: 20 July 2026
bibliography: paper.bib
---

# Summary

Key points to make:

1. Open-source C++ library that improves the quality of planar polygonal meshes.

2. Local, atomic updates; each accepted only when it strictly improves a user-supplied metric; worst entities prioritised.

3. Two primitives: element agglomeration and vertex relaxation.

4. Motivating setting: polygonal meshes arising when tri/quad meshes are cut by embedded interfaces/boundaries.

5. Default metric = element stability ratio; realises the CutVEM agglomeration algorithm and improves VEM conditioning [@vem-cmame].

6. Metric-pluggable, so also a general-purpose polygonal mesh improver.

7. Builds on pmp::SurfaceMesh [@pmp-library]; preserves nodes/DOFs/boundary/subdomains/interfaces; OFF & VTK I/O; CLI app; callbacks.


# Statement of need

Key points to make:

## issues that practitioners actually face

- Polygonal finite elements are mathematically mature (VEM, polygonal FEM) yet largely absent from mainstream commercial/open-source simulation software.
- Practitioners rely on a vast triangle/quad infrastructure, so wholesale adoption of polygonal meshes is neither realistic nor necessary.
- The most common encounter with polygonal elements is involuntary — cutting a tri/quad mesh with an embedded interface or boundary (immersed / fictitious-domain methods) yields general polygons.
VEMesh is built for precisely this setting.

## the obstacle and the principled remedy
- Cut cells are typically slivers/needles → severely ill-conditioned stiffness matrices, a documented obstacle in embedded methods [@dePrenter:2017:CNA; @Schillinger:2015:TFC].
- VEM is the natural method on these meshes (polygon-native, no remeshing, stays conforming) but is still vulnerable to cut-cell ill-conditioning.
- Polygonal quality control has relied on heuristic shape indicators with little consensus; the shape↔conditioning link is tenuous [@sorgente2023survey].
- VEMesh replaces heuristics with a single principled indicator — the element stability ratio (inverse of the element-level condition number) — method-aware and shape-agnostic [@vem-cmame].

## justification for vemesh: why now, for who, and future
- The idea has already been pursued by several independent groups [@Sukumar:2022:AVE; @Fu:2025:PVE; @sorgente2023mesh; @Sorgente:2024:MOV] — validating it, but every implementation so far is bespoke, making a reusable one timely.
- The two primitives generalise the familiar vertex-smoothing and edge-collapse/merging operations, but are applied toward a principled objective (an update is accepted only when it provably improves the chosen metric).
- VEMesh's vertex relaxation generalises provably-robust directional vertex relaxation [@rangarajan2017provably] from triangles to polygons.
- VEMesh clarifies which primitive to use: agglomeration for embedded conditioning (preserves node positions and interface conformity while repairing the element spectrum); relaxation for geometric quality where nodes may move.
- This condition-driven criterion is distinct from shape-metric agglomeration [@Sorgente:2024:MOV].
- Audience: researchers in embedded/immersed FEM and VEM and in moving-boundary problems; VEMesh is pmp-pluggable [@pmp-library], usable as a preprocessing step — the interfaceable implementation the CutVEM study anticipated [@vem-cmame].
- concrete target problems: crack propagation, multimaterial interfaces, and shape and topology optimisation.
- Forward look: currently restricted to planar meshes, but the 2D primitives/metrics/provable-improvement machinery lay the groundwork for a future 3D (polygonal→polyhedral) extension, where cut-cell slivers are most severe.


# Functionality and scope

# Testing and validation


# Acknowledgements

# References
