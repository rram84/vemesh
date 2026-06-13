# Contributing to VEMesh

Contributions are welcome — bug reports, documentation fixes, and code. Please
keep interactions respectful and constructive.

VEMesh improves planar polygonal meshes via local element agglomeration and
vertex relaxation. It is **not** a mesh generator or repair tool. See
[About](docs/about.md) for the full scope before proposing a feature.

## Reporting issues

Please open an issue on the [issue tracker](<repo-url>/issues). For bugs, include
a minimal mesh and code snippet (or `vemesh_app` command) that reproduces the
problem, your platform and compiler, and the full error or incorrect output.

## Seeking support

For questions and help, open an issue on the [issue tracker](<repo-url>/issues).

## Contributing code

For anything beyond a small fix, **open an issue first** to discuss the approach.
Then:

1. Build and test:
   ```sh
   cmake -S . -B build -DBUILD_TESTS=ON && cmake --build build -j && ctest --test-dir build
   ```
2. Work on a topic branch; match the style of the surrounding code; add a test
   for each fix or feature.
3. Open a pull request describing what changed and why, with all tests passing.

By contributing, you agree your contributions are licensed under the project's
MIT License (see [`LICENSE.txt`](LICENSE.txt)).
