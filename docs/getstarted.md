# Get Started {#getstarted}

[TOC]

### Dependencies ###
- C++20 compiler (>=GCC 11.4.0, >= AppleClang 14.0.0)

- [CMake](https://cmake.org) 3.17 or later 
   - Mac: `brew install cmake`
   - Ubuntu: `sudo apt install cmake`
  
    
- [Boost](https://www.boost.org) 1.89 or later
   - Mac: `brew install boost`
   - Ubuntu: `sudo apt install libboost-all-dev`)

- [PMP](https://www.pmp-library.org) Polygon Mesh Processing Library version 3.0
   - Clone, build, and install
  
  ```sh
  git clone https://github.com/pmp-library/pmp-library.git
  cd pmp-library && mkdir build && cd build && cmake .. && make
  sudo make install
  ```
    - [Customize](https://github.com/pmp-library/pmp-library/blob/main/docs/installation.md) your PMP installation

- [Eigen](https://libeigen.gitlab.io)
   - Mac: `brew install eigen`
   - Ubuntu: `sudo apt install libeigen3-dev`

- [Doxygen](https://www.doxygen.nl) Optional
   - Mac: `brew install doxygen`
   - Ubuntu: `sudo apt-get install doxygen graphviz`
   
---

### Build and install VEMesh

**Clone the repository**
```sh
git clone git@bitbucket.org:rram/vemesh.git
```

**Configure and build**
```sh
cd vemesh && mkdir build && cd build && cmake .. && make -j
```
**Install (to a default location)**
```sh
sudo make install 
```
**Run test cases (recommended)**
```sh
ctest -j
```

**Build options**

 - Specify a custom installation path
```sh
cmake ../ -DCMAKE_INSTALL_PREFIX=destination-folder
```

 - If you installed PMP to a custom location, let CMake know where to find it
```sh
cmake ../ -DCMAKE_MODULE_PATH=path/to/package
```

 - Build without unit tests (default=ON)
```sh
cmake ../ -DBUILD_TESTS=OFF
```

 - Build documentation (default=OFF, requires  [Doxygen](https://doxygen.nl))
```sh
cmake ../ -DBUILD_DOCS=ON
make docs
```

---

### Tutorial 
Tutorial-style examples discussed in the @ref tutorial page are provided in the `tutorial/` folder of the repository. 
The tutorial is built as an independent project.

Build and use these examples **after** installing VEMesh:
```sh
cd tutorial && mkdir build && cd build && cmake ../ 
make -j
```

Run the examples:
```sh
./hello_tryumph
```

---

### Try out VEMesh on a mesh
It is helpful to test out element improvement possible with VEMesh before integrating it with your project. 
The application `app/vemesh` is provided for this purpose. 

As discussed in @app, the application can be conveniently used from the command line with flags.
It is built as an independent project. 
```sh
cd app && mkdir build && cd build && cmake ../ 
make
```
Try it out:
```sh
./vemesh -a -r -q -f -i -o
```

---

### Link VEMesh to a project

Find the VEMesh package:
```
# add to CMakeLists.txt
find_package(VEMesh)
```
- if VEMesh is installed in a custom location, use the `CMAKE_MODULE_PATH` flag to specify the path
- `find_package(vemesh)` will automatically look for its dependencies (Boost, Eigen and PMP).

Result variables:  `VEMESH_INCLUDE_DIRS, VEMESH_LIBRARY_DIRS, VEMESH_LIBRARIES`

Link VEMesh to the target:
```
# add to CMakeLists.txt
target_include_directories(TARGET PUBLIC ${VEMESH_INCLUDE_DIRS})
target_link_directories(TARGET PUBLIC ${VEMESH_LIBRARY_DIRS})
target_link_libraries(TARGET PUBLIC ${VEMESH_LIBRARIES})
```

---
