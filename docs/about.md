# VEMesh {#aboutvemesh}
*A quality-driven mesh improvement library for planar polygonal meshes*

[TOC]

## Overview

**VEMesh** is a C++ library for improving the quality of planar
polygonal meshes through small, local, quality-driven updates. It
applies two primitives — **element agglomeration** and **vertex
relaxation** — accepting each only when it strictly improves a
user-supplied quality metric. Poorer-quality faces and vertices are
prioritised.

VEMesh is named for its primary target - the **Virtual Element
Method** - but works as a general-purpose polygonal mesh improver.
See \ref rationale "the rationale" for design motivation.


## Features

VEMesh:
- uses `pmp::SurfaceMesh` from the
  [pmp-library](https://www.pmp-library.org/) as its mesh data
  structure.
- is metric-agnostic; user-defined face quality metrics plug in easily.
- exposes overloaded interfaces giving complete control over scope and
  sequencing of updates.
- applies atomic local updates — one face or one vertex at a time.
- offers callbacks to monitor and visualise updates.
- preserves mesh validity, boundary, subdomains, and embedded interfaces.

## Non-features

VEMesh:
- is not a polygonal mesh generator.
- does not repair invalid meshes — it only improves valid ones.
- has no GUI for visualisation or interactivity.
- is not parallelised and may not be thread-safe.
- is restricted to planar meshes.



## Next steps

- \ref getstarted "Install VEMesh" — dependencies and build.
- \ref tutorial "Tutorials" — worked examples explaining each operation and the key concepts behind them.
- \ref tutorial_app "Command-line app" — try VEMesh on your own mesh without writing code.
- \ref userguide "User guide" — quality metrics, meshes, and algorithmic details.

