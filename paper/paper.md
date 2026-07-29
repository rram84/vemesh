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

VEMesh is an open-source C++ library for improving the quality of planar polygonal meshes. It provides two primitives for local and atomic updates---element agglomeration, which merges a face with a neighboring one, and vertex relaxation, which repositions a vertex. Both operations are applied in a prioritized manner to favor improving the poorest elements first, and each update is accepted only if it strictly improves a user-supplied quality metric. VEMesh is directly aimed at improving polygonal meshes that arise from embedding interfaces or boundaries in triangle or quadrilateral meshes. The library's element stability ratio metric and agglomeration algorithm realize the CutVEM method [@vem-cmame] proposed to address the issue of ill-conditioning in virtual element discretizations over embedded meshes. More generally, VEMesh is designed to incorporate user-defined quality metrics and hence serves as a general-purpose polygonal mesh improver. VEMesh adopts the widely used mesh data structure of the PMP library [@pmp-library], provides utilities to embed geometries in non-conforming meshes, callbacks for monitoring updates, and ships with a command-line application for easy use.

# Statement of need

Polygonal finite element (FE) methods are well-established in the literature. Yet, their adoption in mainstream commercial solid mechanics codes and routine FE simulations remains low. This is in part due to the robust meshing and simulation infrastructure available for triangle and quadrilateral meshes. It is unrealistic to expect polygonal elements, despite their seeming generality, to replace existing simulation practice. There is, however, one decisive scenario in which polygonal elements are inevitable---when embedding boundaries or interfaces in non-conforming meshes, since splitting triangles and quadrilaterals yields polygonal cut cells. The virtual element method (VEM) [@Beirao:2013:BPV] is a natural choice in this setting, and VEMesh is built precisely to aid in this context.

It is well known that cut cells in embedded meshes cause severe ill-conditioning in the global stiffness matrix [@dePrenter:2017:CNA]. This is the case with the VEM too, where the issue is particularly troublesome because of the tenuous link between element shapes and condition numbers in the method. Hence, efforts to control polygonal mesh quality by relying on shape-based metrics [@sorgente2023survey] do not address the problem. VEMesh provides a method-aware and shape-agnostic solution through the element stability ratio metric [@vem-cmame], element agglomeration and vertex relaxation. VEMesh is hence *not* a polygonal mesh generator or an implementation of the VEM; rather, it provides a pre-processing tool to address ill-conditioning in the VEM on embedded meshes while only computing local element matrices and leaving degrees of freedom unchanged. The library will directly aid practitioners of the VEM and researchers working with embedded methods for simulating moving boundary problems such as crack propagation, multimaterial interfaces, and shape/topology optimization.

# State of the field

The problem of embedding geometries in non-conforming meshes has motivated several FE methods in the literature. As a representative few, we mention immersed boundary methods [@Peskin:2002:TIB], the extended finite element method [@Moes:1999:AFE], CutFEM [@Burman:2015:CDG], CutIGA [@Elfverson:2018:CUT], and the shifted boundary method [@Main:2018a:SBM], as well as the related ideas of ghost penalty [@Burman:2010:GP] and specialized preconditioners for ill-conditioning [@dePrenter:2017:CNA]. The merits of these methods notwithstanding, their adoption in general-purpose codes remains limited, and integrating such specialized formulations into existing workflows is cumbersome. In contrast, VEMesh does not implement a new method. Rather, it addresses ill-conditioning through mesh processing that is easily integrated into existing workflows.

In the context of tools for working with polygonal meshes, two requirements are well-addressed in the literature. First, algorithms and codes to generate high-quality polygonal meshes are available [@polymesher].  Second, mesh processing libraries [@pmp-library; @cgal] provide data structures for working with polygonal meshes.  However, algorithms and libraries for improving general polygonal meshes are not commonplace and usually only cater to triangle/quadrilateral meshes. Dedicated codes implementing the VEM  [@veamy; @vempp] are available but do not address intricacies arising from cut cells. In this sense, VEMesh addresses an important gap.


We also note the works [@sorgente2023survey;@sorgente2023mesh;@Sorgente:2024:MOV] that explore geometric quality metrics for polygons and optimization-based algorithms for element agglomeration. While noting that the stability ratio and local primitives provided by VEMesh differ from these, these recent efforts emphasize the significance of the problem addressed by the library.


# Software design

We highlight a few important considerations in VEMesh's design.  

- **Quality metric specification**: The metric is evaluated through a function that takes only nodal coordinates of a face as input. In particular, the metric has no context of the mesh, neighboring elements, or any state. This makes it trivial for users to implement a custom metric that is safe to invoke concurrently. This feature makes the library metric-agnostic rather than VEM specific; the element stability ratio and the shape ratio provided in the library are two specific instances. 

- **Flexibility in sequencing primitives**: Element agglomeration and vertex relaxation are complementary operations. Their precise sequencing depends on the choice of quality metric and the goal of mesh improvement. With the stability ratio metric, for instance, agglomeration is far more impactful than relaxation in improving matrix conditioning (\autoref{fig:example}). Hence, VEMesh provides complete freedom in sequencing operations, and exposes each operation at three levels of granularity---a single face/vertex, a user-supplied subset, or over the whole mesh by identifying faces with quality falling below a threshold.

![Effect of the mesh-improvement primitives provided by VEMesh on the conditioning of an embedded-mesh problem. Five interfaces are embedded in a fixed triangle mesh (top left). For each geometry, the global stiffness matrix of a prototypical Poisson problem is assembled with the VEM. To generate a rich set of interface-mesh intersections, vertices in the vicinity of each interface are randomly perturbed to produce 1000 distinct embedded-mesh realizations per geometry. The plot compares, for each geometry, the condition number of the global stiffness matrix with the embedded mesh and those computed after applying agglomeration and relaxation while using the stability ratio metric. Further details on the computation are given in the library's documentation. The condition number of the embedded mesh shows a large spread, while including element agglomeration drastically improves (reduces) both the condition number's mean and its spread.\label{fig:example}](fig.png){ width=99% }

- **No solve()**: Mesh updates in VEMesh are local, and individually accepted or rejected to ensure monotonic improvement. The library does not provide a single method that optimizes the mesh in one shot, i.e., it does not attempt to identify a global optimum in any sense. Instead, it provides visibility and control by invoking a user-controlled callback after each mesh update. This allows the mesh to be inspected, its intermediate states to be recorded, and even subsequent operations to be aborted if desired.

- **Vertex relaxation by sampling**: Admissible positions when relocating a vertex are limited by visibility criteria. Experiments showed that computing the required polygon kernels was too expensive. As a remedy, VEMesh generates locations within the bounding box of the incident faces at a vertex and tests each candidate's admissibility. Correctness remains unaffected, since a candidate is accepted only if it strictly improves quality. The cheaper search, however, incurs the expense of generating, checking and rejecting inadmissible candidates. The search is exposed in the relaxation interface by requiring the number of samples to draw and an optional random seed.

- **Limited parallelism**: Read-only quality evaluations, mesh-wide search for elements requiring improvement, and candidate scoring during vertex relaxation are parallelized with OpenMP. Element agglomeration and the order of vertex relaxation are kept sequential, since parallelization would allow concurrent updates. While this is not a limitation of the algorithm implemented, it entails partitioning operations and jeopardizes reproducibility. Besides, we do not expect good scaling with embedded meshes in which just a small fraction of elements near the embedded geometry require improvement. 

- **Mesh data structure**: VEMesh adopts the surface mesh data structures provided by the PMP library [@pmp-library] rather than implementing a custom one. VEMesh associates properties to the mesh to incorporate boundary and interface constraints during improvement. Although the library does not require other functionalities of the PMP library, we think the dependency is worth incurring to improve VEMesh's adoption. 

- **Qualities computed on demand**: Face qualities are never cached, since agglomeration and relaxation continually invalidate them. Recomputing avoids stale-state bugs, and even allows the metric to be changed between operations on the same mesh.

- **Geometry embedding utilities**: The library provides utilities to embed curvilinear and polygonal interfaces/boundaries in triangle/quadrilateral meshes. Even though this is not a core functionality of the library, it is central to its intended use and essential in its evaluation.

# Research impact

VEMesh is the implementation behind CutVEM [@vem-cmame]. The computational results reported there, supporting the shape-agnostic agglomeration algorithm and the stability ratio metric, and underlying the studies on embedded geometries, were all computed with VEMesh. 

VEMesh ships with the mesh generators, run scripts, geometries, a MATLAB script to evaluate condition numbers of global stiffness matrices [@sutton2017vem], and analysis pipelines required to reproduce the studies reported in CutVEM and in the library's documentation pages. A distinctive aspect of these tests is their statistical nature. Since matrix conditioning depends crucially on the specifics of the geometry-mesh intersection, a single realization is neither representative nor informative. The performance tests in the library therefore generate a large collection of mesh-geometry intersections through random perturbations of the background mesh, with the reported studies drawing on 1000 such realizations for each geometry (\autoref{fig:example}). This is a critical requirement for robust numerical simulations. The complete output of these studies is archived and openly available [@vemesh-data]. Documented experiments with the library also demonstrate clear improvements in mesh quality and matrix conditioning for benchmark meshes from recent studies [@sorgente2022role].

Vertex relaxation is, in this context, a new primitive. The CutVEM study [@vem-cmame] only used element agglomeration. VEMesh introduces vertex relaxation as an additional independent operation. Somewhat surprisingly, numerical experiments with VEMesh show that relaxation by itself improves the conditioning only modestly. Its benefit is instead complementary, since pairing vertex relaxation with element agglomeration attains conditioning gains comparable to agglomeration alone while agglomerating fewer elements. With user-defined metrics (e.g., geometric), vertex relaxation can substantially improve mesh quality as well [@rangarajan2017provably]. 

VEMesh is not limited to the VEM or to improving embedded meshes. With customizability of the quality metric and the sequencing of operational primitives, the library is also a tool for general polygonal mesh improvement. The library will also be crucial in evaluating the prospect of three-dimensional extensions of the underlying algorithms. 


# AI usage disclosure

Claude Code was used to set up a Docker image and the CI pipeline, to write the execution scripts for performance testing, to improve the CMake build system, to format mesh I/O, and to assist with the library's documentation.

No algorithmic aspect of the library was implemented with AI assistance. The mesh improvement primitives, the quality metrics, and the data structures on which they operate were designed, implemented, and tested by the authors.

# Acknowledgements

RR gratefully acknowledges financial support from the Anusandhan National Research Foundation (ANRF) through the project CRG/2022/006569.

# References
