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
- is only lightly parallelized. it uses OpenMP to accelerate read-only quality evaluations
  and for candidate scoring in vertex relaxation. In general, public
  APIs are not safe to call concurrently from multiple threads.
- is restricted to planar meshes.


## Next steps

- \subpage rationale "Rationale" — design philosophy and motivation.
- \subpage performance "VEMesh in practice" — what to expect, with benchmark results on real meshes.
- \subpage getstarted "Install VEMesh" — dependencies and build.
- \subpage tutorial "Tutorials" — worked examples explaining each operation and the key concepts behind them.
- \subpage userguide "User guide" — quality metrics, meshes, and algorithmic details.

Try VEMesh on your own mesh without writing code with the
\ref tutorial_app "command-line app".

Interested in contributing? See the [contributing guidelines](../CONTRIBUTING.md).
VEMesh is released under the [MIT License](LICENSE.txt).

