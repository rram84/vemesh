# Get Started {#getstarted}

[TOC]

## Dependencies
- **C++17 compiler** (≥ GCC 11.4.0, ≥ AppleClang 14.0.0)

- [CMake](https://cmake.org) ≥ 3.17
```bash
# macOS
brew install cmake
# Ubuntu
sudo apt install cmake
```
  
- [Boost](https://www.boost.org) ≥ 1.87
```bash
# macOS
brew install boost
# Ubuntu
sudo apt install libboost-all-dev
```


- [PMP](https://www.pmp-library.org) version 3.0
```bash
git clone https://github.com/pmp-library/pmp-library.git
cd pmp-library
mkdir build && cd build
cmake ..
make
sudo make install
```
 Optional: [Customize your PMP installation](https://github.com/pmp-library/pmp-library/blob/main/docs/installation.md)

- [Eigen](https://libeigen.gitlab.io)
```bash
# macOS
brew install eigen
# Ubuntu
sudo apt install libeigen3-dev
```

- **[Doxygen](https://www.doxygen.nl)** (optional)
```bash
# macOS
brew install doxygen
# Ubuntu
sudo apt-get install doxygen graphviz
```

- **[CLI11](https://github.com/CLIUtils/CLI11)** (required when
  `BUILD_TESTS=ON`, optional otherwise)
  ```bash
# macOS (Homebrew)
brew install cli11
# Ubuntu
sudo apt-get install cli11-dev
```

## Build and install vemesh

**Clone the repository:**
```sh
git clone git@bitbucket.org:rram/vemesh.git
```

**Configure and build:**
```sh
cd vemesh
mkdir build && cd build
cmake ..
make -j
```

**Install (default location):**
```sh
sudo make install 
```

**Run tests (recommended):**
```sh
ctest -j
```

### Build options

**Custom installation path:**
```sh
cmake ../ -DCMAKE_INSTALL_PREFIX=path/to/install
```

**Custom PMP location:**
```sh
cmake ../ -DCMAKE_MODULE_PATH=path/to/pmp
```

**Disable unit tests (default: ON):**
```sh
cmake ../ -DBUILD_TESTS=OFF
```
`BUILD_TESTS=ON` also builds the tutorial examples and the in-tree
\ref performance tooling. Required CLI11.

**Build documentation (default: OFF, requires Doxygen):**
```sh
cmake ../ -DBUILD_DOCS=ON
make docs
```

## Tutorial 
Tutorial-style examples discussed in the @ref tutorial page are provided in the `tutorial/` folder of the repository.

These examples are built **as part of the main build** when `BUILD_TESTS=ON` (the default), and
they double as part of the test suite (`ctest`). No separate configuration or installation step
is needed — building `vemesh` above also builds the tutorials.

Each tutorial produces its own executable (`element_agglomeration`, `vertex_relaxation`,
`custom_quality_metric`, …) under `build/tutorial/`, alongside the sample input meshes copied
to `build/tutorial/sample_data/`. Run one from there, for example:
```sh
cd build/tutorial
./element_agglomeration
```


## Try  vemesh
Test the element improvement possible with `vemesh` before integrating it with your project,
using the \ref tutorial_app "vemesh_app" command-line tool. Its source is `app/vemesh_app.cpp`.

`vemesh_app` is a **standalone CMake project** in the `app/` folder. It consumes the installed
`vemesh` package and uses [CLI11](https://github.com/CLIUtils/CLI11) for command-line parsing,
so build it **after** installing `vemesh` (and CLI11).

**Build the app:**
```bash
cd app
mkdir build && cd build
cmake ..        # add -DCMAKE_PREFIX_PATH=path/to/install for a custom prefix
make
```

**Run with example flags:**
```bash
./vemesh_app -a -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -m stability
```

See the \ref tutorial_app page for the full list of flags, options, and usage examples.

## Link vemesh to your project

**Find the package in CMake:**
```cmake
find_package(vemesh REQUIRED)
```

**Link to your target:**
```cmake
target_link_libraries(your_target PUBLIC vemesh::vemesh)
```

> `vemesh::vemesh` is an interface target that aggregates all components:
> - `vm_utils`
> - `vm_io`
> - `vm_quality`
> - `vm_optimizer`
 
- Headers are installed in `vemesh/include` 
- Include headers like:
```cpp
#include <vemesh/vm_mesh_optimizer.h>
```

