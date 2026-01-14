# VEMesh {#aboutvemesh}

[TOC] 

*A quality-driven mesh improvement library for planar polygonal meshes*

**Author:**  
Ramsharan Rangarajan  
Associate Professor of Mechanical Engineering  
Indian Institute of Science (IISc), Bengaluru, India

## Overview  

**VEMesh** is a C++ library for improving the quality of planar
polygonal meshes through **quality-driven local updates**.  

The library provides two mesh modification operations— **element
agglomeration** and **vertex relaxation** —both of which are accepted
only when they provably improve a user-supplied quality metric.  Both
operations prioritize poorer elements and vertices to systematically
improve mesh quality while avoiding ad hoc
modifications and expensive non-local optimization.  

Though effective as a general-purpose tool for improving polygonal
meshes, the library is named for its intended purpose— improving the
robustness and performance of the **Virtual Element
Method (VEM)**.  

For the motivation behind the library, see \ref rationale "the rationale for VEMesh".

## Features (and non-features)  
We highlight a few key features of the library. VEMesh:  
- leverages the widely-used pmp::SurfaceMesh data structure from the
  [pmp-library](pmp-library.org) for representing meshes.  
- is agnostic to the quality metric employed.   
- enables convenient implementation of user-defined face quality
  metrics. 
- provides simple overloaded interfaces for agglomeration and relaxation
  operations that enable complete control over the purpose and
  sequence of operations.  
- implements local mesh updates. The atomic operations provided consider one
  face or one vertex at a time. Updates are hence fast and efficient.  
- implements callbacks to monitor and visualize mesh updates.  
- preserves mesh validity. Mesh improvement operations
  never result in degenerate or tangled faces.  
- preserves the mesh boundary, subdomains, and embedded
  interfaces.   

It is also important to note what the library is **not meant for**. VEMesh:  
- is not a polygonal mesh generator.  
- is not a tool for mesh repair. It only improves valid meshes.  
- does not guarantee mesh improvement, although it generally
  achieves significant improvement.  
- does not include a GUI for visualization/interactivity.
- is not parallelized and may not be thread-safe.
- is restricted to planar meshes.

## Getting started  
- Check out a simple \ref tutorial_element_agglomeration "tutorial-style example"
  showing how the library works and how it can be used.
- \ref getstarted "Get the dependencies and install VEMesh."  
- Test VEMesh on your mesh from the \ref tutorial_app "command-line".  
- Read the \ref userguide "user guide".  


