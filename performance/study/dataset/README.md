# Study geometry dataset (frozen)

The fixed input for the relaxation-vs-agglomeration study. **Do not recompute on
the fly** — these files are the canonical, version-controlled study input. The
one-time tooling that produced them lives in `../geometry/`.

## Contents
- `geometries/*.dat` — 527 closed polygonal interfaces. One `x y` sample per
  line; a **blank line separates one closed loop from the next** (multi-component
  shapes). Read with `vm::tutorial::read_polygon_loops`.
- `backgrounds/bg_tri_128.off`, `bg_quad_128.off` — the two fixed structured
  background meshes over `[-1,1]^2` at N=128 (h = 2/128 ≈ 0.0156). Triangle and
  quad share the same nodes.
- `manifest.csv` — `stem,source,change` per shape: `raw` (unmodified) or
  `close@<delta>` (morphologically coarsened) with its Jaccard shape-change;
  `dropped` rows are listed for provenance but have no `.dat`.

## Provenance / processing
1. Source: 578 piecewise-Bezier feature sets (`.fset`: line / rational-quadratic
   / cubic segments). 47 with open components excluded → 531 closed.
2. Oversampled to polygons (`fset_to_polygon`), then **scaled aspect-preserving
   so each shape's larger extent = 1.8 and centered at the origin** (spans
   `[-0.9,0.9]`, ~115 cells across at N=128, 0.1 margin to the domain edge).
3. Admissibility (`sampling_check`): a shape is admissible iff **no background
   cell is crossed by two distinct interface loops** (inter-component; single-loop
   self-necks/corners are tolerated — they embed validly, the SDF just rounds them
   locally). Quad is the binding case (tri shares its nodes).
4. Residual failures salvaged (`coarsen_geometry --mode close`) by morphological
   closing at delta = h then 2h (merges nearby loops / fills sub-cell gaps; never
   erodes features), accepted only if Jaccard shape-change <= 0.2, else dropped.

Result: **527 / 531** (521 raw + 6 coarsened; 4 dropped).

## Regenerate (if ever needed)
Build the tooling in `../geometry/` and run, e.g.:
```
fset_to_polygon --batch -i <fset_dir> -o <out> --fit 1.8
make_background -n 128 -t both -o <bg_dir>
# then the finalize pass (sampling_check + coarsen_geometry, see git history)
```
