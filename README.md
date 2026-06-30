# VEMesh

*A quality-driven mesh improvement library for planar polygonal
meshes*

**Full documentation:** [rram.bitbucket.io/vemesh-docs][docs]

Pages: [About][about] · [Rationale][rationale] · [Get Started][getstarted] · [Tutorials][tutorials] · [User Guide][userguide] · [VEMesh in practice][practice]

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

VEMesh is **not** a mesh generator or repair tool, is only lightly parallelised
(OpenMP for read-only quality evaluation and relaxation candidate scoring), and
is restricted to planar meshes. The full feature list and non-features are on the
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
git clone https://bitbucket.org/rram/vemesh.git
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

Full documentation is generated with Doxygen
(`cmake -S . -B build -DBUILD_DOCS=ON && cmake --build build --target docs`)
and hosted at **[rram.bitbucket.io/vemesh-docs][docs]**. Main entry points:

- [About][about] — overview and feature list
- [Rationale][rationale] — design philosophy and motivation
- [Get Started][getstarted] — dependencies, build, and install
- [Tutorials][tutorials] — worked examples for each operation
- [User Guide][userguide] — quality metrics, conventions, and algorithmic details
- [VEMesh in practice][practice] — benchmark results on real meshes

## Contributing

Bug reports, feature requests, and pull requests are welcome via the
[issue tracker](https://bitbucket.org/rram/vemesh/issues). See
[`CONTRIBUTING.md`](CONTRIBUTING.md) for guidelines, and please run `ctest`
locally before submitting a PR.

## License and citation

VEMesh is released under the MIT License; see [`LICENSE.txt`](LICENSE.txt).
If you use VEMesh in your work, please cite *(TBA)*.

[docs]:       https://rram.bitbucket.io/vemesh-docs/
[about]:      https://rram.bitbucket.io/vemesh-docs/index.html
[rationale]:  https://rram.bitbucket.io/vemesh-docs/rationale.html
[getstarted]: https://rram.bitbucket.io/vemesh-docs/getstarted.html
[practice]:   https://rram.bitbucket.io/vemesh-docs/performance.html
[userguide]:  https://rram.bitbucket.io/vemesh-docs/userguide.html
[tutorials]:  https://rram.bitbucket.io/vemesh-docs/tutorial.html
[cmake]:      https://cmake.org
[boost]:      https://www.boost.org
[eigen]:      https://libeigen.gitlab.io
[pmp]:        https://www.pmp-library.org
[cli11]:      https://github.com/CLIUtils/CLI11
[doxygen]:    https://www.doxygen.nl
