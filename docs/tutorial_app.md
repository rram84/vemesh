
\page tutorial_app Command-line tool: vemesh_app

`vemesh_app` is a command-line utility to perform *iterative* 
element agglomeration and vertex relaxation operations on polygonal
meshes using the vemesh library.  
It is provides to help conveniently evaluate the capabilities of the
vemesh library with user data.  
It supports OFF and VTK mesh formats, and provides options for quality
metrics, iteration counts, and mesh output.

**Source code:** app/vemesh_app.cpp

\note `vemesh_app` is built as a **standalone CMake project** under `app/`, separate from the
main library and the tutorial examples. Build it after installing `vemesh` — see the
\ref getstarted page.

**Overview:**  

- Specify an input polygonal mesh
- Select an optimization mode: agglomeration, vertex relaxation, or a combination of both.
- Specify corresponding algorithmic parameters.
- Choose a face quality metric.
- For a prescribed number of iterations:
  - Apply agglomeration and/or vertex relaxation in the specified order.
  - Update face and vertex quality measures after each iteration.
- Write mesh outputs according to the selected verbosity level.
- Save the final optimized mesh with updated quality annotations.

[TOC] 

 The application uses **CLI11** for command-line parsing, provided as a single header in `external/` so no separate install is needed.

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
- `-S <value>` : RNG seed for reproducible vertex relaxation (optional; default: nondeterministic)
- `-f <value>` : Quality improvement factor (for agglomeration)
- `-m <value>` : Face quality metric
  - `stability` : Element stability ratio
  - `shape`  : Shape quality
- `-v <mode>`   : Mesh output mode
  - `none` : no output (default)
  - `iter` : output at the end of each iteration
  - `detailed` : output after each update within iteration

\note Vertex relaxation perturbs vertices using randomly sampled candidate
positions, so `-r`, `--ar`, and `--ra` runs are nondeterministic by default.
Pass `-S <seed>` to fix the RNG seed and reproduce a run exactly. The seed has no
effect on `-a` (agglomeration is deterministic).


## Parallelism
`vemesh_app` performs its quality evaluations and mesh-inspection checks through
the vemesh library, which parallelizes those read-only loops with **OpenMP** when
available (see \ref ug_quality_parallel and the utilities user guide). Vertex
relaxation additionally scores each vertex's candidate positions in parallel.
Element agglomeration and the order in which vertices are relaxed remain
sequential, so the result is unchanged by the thread count.

Control this with the standard OpenMP environment variable:
```
OMP_NUM_THREADS=4 ./vemesh_app -a -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -m stability
```
If vemesh (and `vemesh_app`) were built without OpenMP, the count is always `1`.


## Command-Line Usage

| Mode | Description | Required Options | Optional Flag  |
|------------|-----------------|---------------|-------------------|
| `-a`       | Agglomerate elements only.                     |`-i`, `-o`, `-n`, `-q`, `-m`, `-f`            | `-v` | 
| `-r`       | Relax vertices only.                                  |`-i`, `-o`, `-n`, `-q`, `-m`, `-s`            | `-v`, `-S` | 
| `--ar`    | Agglomerate then relax in each iteration.  |`-i`, `-o`, `-n`, `-q`, `-m`, `-f`, `-s` | `-v`, `-S` | 
| `--ra`    | Relax then agglomerate in each iteration.  |`-i`, `-o`, `-n`, `-q`, `-m`, `-f`, `-s` | `-v`, `-S` | 


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
  A mesh is written once at the end of each iteration as `mesh-iter-<k>.vtk`

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

For example, `mesh-iter-4-r-3.vtk` is the mesh written during the relaxation (`r`) step of iteration 4, after its 3rd internal update.

## Examples

**Agglomerate elements:**
```
./vemesh_app -a -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -m stability -v detailed
```

**Relax vertices (reproducible with a fixed seed):**
```
./vemesh_app -r -i in_mesh.OFF -o out_dir -n 5 -s 5 -m shape -v iter -S 42
```

**Agglomerate and relax:**
```
./vemesh_app --ar -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5 -m stability -v iter
```

**Relax and agglomerate:**
```
./vemesh_app --ra -i in_mesh.OFF -o out_dir -n 5 -f 1.2 -s 5 -m shape -v none
```


