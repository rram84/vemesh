# Get Started {#getstarted}

[TOC]

## Dependencies
- **C++20 compiler** (≥ GCC 11.4.0, ≥ AppleClang 14.0.0)

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

- **[CLI11](https://github.com/CLIUtils/CLI11)** (optional, required by \ref app "vemesh_app" for command-line parsing)
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

**Build documentation (default: OFF, requires Doxygen):**
```sh
cmake ../ -DBUILD_DOCS=ON
make docs
```

## Tutorial 
Tutorial-style examples discussed in the @ref tutorial page are provided in the `tutorial/` folder of the repository. 
The tutorial is built as an independent project.

Build and use these examples **after** installing `vemesh`:
```sh
cd tutorial && mkdir build && cd build && cmake ../ 
make -j
```

Run the examples:
```sh
./hello_tryumph
```


## Try  vemesh
Test element improvement possible with `vemesh` before integrating it with your project.  
Use the \ref app application `app/vemesh`.
 
**Build the app:**
```bash
cd app
mkdir build && cd build
cmake ..
make
```

**Run with example flags:**
```bash
./vemesh -a -r -q -f -i -o
```

The app uses [CLI11](https://github.com/CLIUtils/CLI11) for command
line parsing. The header is provided in the `app/` folder.

## Link vemesh to Your Project

**Find the package in CMake:**
```cmake
find_package(VEMesh REQUIRED)
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

