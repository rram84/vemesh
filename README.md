# VEMesh

*A quality-driven mesh improvement library for planar polygonal
meshes*

<!-- badges: build status, docs, license, JOSS DOI go here once available -->

VEMesh improves planar polygonal meshes through **local, atomic, quality-driven
updates**. It provides two mesh-modification primitives — **element
agglomeration** and **vertex relaxation** — that are accepted only when they
provably improve a user-supplied quality metric. Poorer-quality entities are
prioritised, so computational effort is focused where it matters most.

The library is named for its primary application: improving the robustness and
performance of the **Virtual Element Method (VEM)**, which is naturally suited
to polygonal discretisations. It is also useful as a general-purpose polygonal
mesh-improvement tool. See the [rationale][rationale] for the motivation
behind the library.

## Highlights

- Operates on `pmp::SurfaceMesh` from the [pmp-library][pmp].
- User-defined polygon quality metrics are easy to plug in.
- Two atomic operations with overloaded interfaces for full control over scope and sequencing.
- Preserves mesh validity, boundary, subdomains, and embedded interfaces.
- Callbacks for monitoring and visualising updates.
- Reads/writes OFF and VTK.

VEMesh is **not** a mesh generator or repair tool, is not parallelised, and is
restricted to planar meshes. The full feature list and non-features are on the
[About][about] page.

## Dependencies

Required:

- C++20 compiler (GCC ≥ 11.4 or AppleClang ≥ 14)
- [CMake][cmake] ≥ 3.17
- [Boost][boost] ≥ 1.87
- [Eigen][eigen]
- [PMP][pmp] 3.0

Optional:

- [CLI11][cli11] — required only by the command-line app
- [Doxygen][doxygen] (with Graphviz) — required only to build the documentation

Platform-specific install commands are listed on the [Get Started][getstarted] page.

## Installation

```sh
git clone <github-url>/vemesh.git
cd vemesh
cmake -S . -B build -DBUILD_TESTS=ON
cmake --build build -j
ctest --test-dir build -j
sudo cmake --install build
```

To link from your own CMake project:

```cmake
find_package(vemesh REQUIRED)
target_link_libraries(your_target PUBLIC vemesh::vemesh)
```

## Getting started

- [Tutorials][tutorials] walk through each operation with runnable examples in [`tutorial/`](tutorial/).
- The [command-line app](app/vemesh_app.cpp) lets you try VEMesh on your own mesh without writing code.
- The [User Guide][userguide] covers quality metrics, conventions, and algorithmic details.

## Documentation

Full documentation is generated with Doxygen (`cmake -DBUILD_DOCS=ON && make docs`)
and hosted at **[\<docs-url\>][docs]**. Main entry points:

- [About][about] — overview and feature list
- [Rationale][rationale] — design philosophy and motivation
- [Get Started][getstarted] — dependencies, build, and install
- [User Guide][userguide] — quality metrics, conventions, and algorithmic details
- [Tutorials][tutorials] — worked examples for each operation

## Contributing

Bug reports, feature requests, and pull requests are welcome via the
[issue tracker](<github-url>/vemesh/issues). See
[`CONTRIBUTING.md`](CONTRIBUTING.md) for guidelines, and please run `ctest`
locally before submitting a PR.

## License and citation

VEMesh is released under the MIT License; see [`LICENSE.txt`](LICENSE.txt).
If you use VEMesh in academic work, please cite *(JOSS paper / DOI to be added)*.

[docs]:       https://<docs-host>/vemesh/
[about]:      https://<docs-host>/vemesh/aboutvemesh.html
[rationale]:  https://<docs-host>/vemesh/rationale.html
[getstarted]: https://<docs-host>/vemesh/getstarted.html
[userguide]:  https://<docs-host>/vemesh/userguide.html
[tutorials]:  https://<docs-host>/vemesh/tutorial.html
[cmake]:      https://cmake.org
[boost]:      https://www.boost.org
[eigen]:      https://libeigen.gitlab.io
[pmp]:        https://www.pmp-library.org
[cli11]:      https://github.com/CLIUtils/CLI11
[doxygen]:    https://www.doxygen.nl
