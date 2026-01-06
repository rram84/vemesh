
\page tutorial_app Command-line tool: vemesh_app

[TOC] 

`vemesh_app` is a command-line utility to perform *iterative* 
element agglomeration and vertex relaxation operations on polygonal
meshes using the vemesh library.  
It is provides to help conveniently evaluate the capabilities of the
vemesh library with user data.  
It supports OFF and VTK mesh formats, and provides options for quality
metrics, iteration counts, and mesh output.

**Source code:** vemesh_app.cpp

 The application uses **CLI11** for command-line parsing. Check the \ref getstarted page for install instructions.


## Flags

Choose exactly one optimization mode:

- `-a` : Agglomeration only
- `-r` : Vertex relaxation only
- `--ar` : Agglomeration followed by vertex relaxation
- `--ra` : Vertex relaxation followed by agglomeration


## Options

- `-i <file>` : Input polygonal mesh file (OFF/VTK format)
- `-o <dir>` : Output directory (will be created or cleared)
- `-q <value>` : Minimum acceptable element quality
- `-n <value>` : Number of iterations to perform
- `-s <value>` : Number of vertex samples (for vertex relaxation)
- `-f <value>` : Quality improvement factor (for agglomeration)
- `-m <value>` : Face quality metric
  - `1` : Element stability ratio
  - `2` : Shape quality
  - `3` : Minimum angle
- `-v <mode>` : Mesh output mode
  - `none` : no output (default)
  - `iter` : output at the end of each iteration
  - `detailed` : output after each update within iteration


## Command-Line Usage

| Mode | Description | Required Options | Optional Flag  |
|------------|-----------------|---------------|-------------------|
| `-a`       | Agglomerate elements only.                     |`-i`, `-o`, `-n`, `-q`, `-m`, `-f`            | `-v` | 
| `-r`       | Relax vertices only.                                  |`-i`, `-o`, `-n`, `-q`, `-m`, `-s`            | `-v` | 
| `--ar`    | Agglomerate then relax in each iteration.  |`-i`, `-o`, `-n`, `-q`, `-m`, `-f`, `-s` | `-v` | 
| `--ra`    | Relax then agglomerate in each iteration.  |`-i`, `-o`, `-n`, `-q`, `-m`, `-f`, `-s` | `-v` | 


## Output 
The application always writes the **initial** and **final** meshes to file, independent of the selected output mode.

- **Initial mesh**  
  Saved as `input_mesh.vtk` immediately after loading the input mesh and evaluating initial quality measures.  
  Face qualities are saved under the property `face_quality`.  
  Vertex qualities are saved under the property `vertex_quality`.  


- **Final mesh**  
  Saved as `output_mesh.vtk` after completion of all optimization iterations and final quality evaluation.  
  Face qualities are saved under the property `face_quality`.  
  Vertex qualities are saved under the property `vertex_quality`.  


Additional mesh outputs depend on the selected output mode (`-v`):  

- `none`  
  Only the initial and final meshes are written.

- `iter`  
  A mesh is written once at the end of each iteration as mesh-iter-<k>.vtk

- `detailed`  
  Meshes are written after every mesh update during agglomeration
  and/or relaxation.  
  This can result in a large number of output files and should be used
  sparingly.  
  The application writes mesh files from within the progress callback during agglomeration and/or relaxation.  
  Each file name encodes:  
  - the iteration number
  - the operation being performed
  - the update count within that operation  
  The general filename pattern is: `mesh-iter-<iter>-<operation>-<update>.vtk`, where:  
  - `<iter>`  
	Zero-based outer iteration index.
  - `<operation>`  
  Short descriptor indicating the optimization step:
     - `a`   : agglomeration
     - `r`   : relaxation
     - `a-r` : relaxation following agglomeration
     - `r-a` : agglomeration following relaxation
  - `<update>`  
  Number of completed internal updates within the current operation
  (as reported by `vm::ProgressInfo::num_completed`).  

For example, `mesh-iter-4-r-3.vtk` is the

## Examples

**Agglomerate elements:**
```
./vemesh -a -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -v
```

**Relax vertices:**
```
./vemesh -r -i in_mesh.OFF -o out_dir -n 5 -s 5
```

**Agglomerate and relax:**
```
./vemesh --ar -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5 -v
```

**Relax and agglomerate:**
```
./vemesh --ra -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5
```


