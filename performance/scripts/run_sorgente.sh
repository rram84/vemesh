#!/usr/bin/env bash
# Sriramajayam
#
# Performance test: sorgente meshes.
# For each sample_data/sorgente/*.off mesh, run vemesh_app three ways
#   - agglomeration only          (-a)
#   - relaxation only             (-r)
#   - agglomerate + relax (alt.)  (--ar)
# and stage baseline + improved meshes as VTK for the MATLAB VEM-conditioning
# evaluation in performance/matlab/.
#
# Output layout (mesh-first):
#   performance/output/sorgente/<name>/{baseline,agglomerate,relax,agglomerate_relax}.vtk
# In MATLAB, evaluate_directory(<name>) returns a 4-row table comparing the
# variants for that mesh.

set -euo pipefail
shopt -s nullglob

# --------------------------------------------------------------------------- #
# Configuration  (edit the meshes and algorithm options below)
# --------------------------------------------------------------------------- #

# meshes to process; empty => all *.off in $SORGENTE. Override with basenames:
#   MESHES=(mesh1_20 mesh2_20 mesh3_20)
MESHES=()

# Full vemesh_app option string per variant (everything except -i/-o, which the
# driver supplies). Include -m here; add -S <seed> on the relaxation variants for
# reproducibility (seed has no effect on agglomeration-only).
AGG_OPTS="-n 5 -q 0.2 -f 1.2 -m stability"                  # agglomeration only            (-a)
RELAX_OPTS="-n 5 -q 0.2 -s 20 -m stability -S 12345"        # relaxation only               (-r)
AR_OPTS="-n 5 -q 0.2 -f 1.2 -s 20 -m stability -S 12345"    # alternate agglomerate + relax (--ar)

# --------------------------------------------------------------------------- #
# Paths  (no edits needed; resolved relative to this script)
# --------------------------------------------------------------------------- #

# This script and its inputs/outputs all live under build/performance/ (copied
# there by CMake). Run it from there; everything resolves relative to $HERE,
# and outputs land under ./output (covered by the existing build/ gitignore).
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

VEMESH_APP="$HERE/vemesh_app"            # built alongside this script
SORGENTE="$HERE/sample_data/sorgente"    # copied in by CMake
OUT="$HERE/output/sorgente"

# --------------------------------------------------------------------------- #
# Driver  (no edits needed below)
# --------------------------------------------------------------------------- #

[[ -x "$VEMESH_APP" ]] || { echo "error: vemesh_app not found at $VEMESH_APP (build with -DBUILD_TESTS=ON)"; exit 1; }
[[ -d "$SORGENTE"   ]] || { echo "error: sorgente dir not found at $SORGENTE"; exit 1; }

if [[ ${#MESHES[@]} -eq 0 ]]; then
  for f in "$SORGENTE"/*.off; do MESHES+=("$(basename "$f" .off)"); done
fi
[[ ${#MESHES[@]} -gt 0 ]] || { echo "error: no .off meshes found in $SORGENTE"; exit 1; }

mkdir -p "$OUT"
MANIFEST="$OUT/manifest.csv"
echo "mesh,variant,command" > "$MANIFEST"

# run_variant <name> <variant> <mode-flag> <opts>
run_variant() {
  local name="$1" variant="$2" mode="$3" opts="$4"
  local run="$OUT/_runs/$name/$variant"
  rm -rf "$run"; mkdir -p "$run"

  local cmd=("$VEMESH_APP" "$mode" -i "$SORGENTE/$name.off" -o "$run" $opts)
  echo "  [$variant] ${cmd[*]}"
  "${cmd[@]}"

  cp "$run/vtk/output_mesh.vtk" "$OUT/$name/$variant.vtk"
  echo "$name,$variant,\"${cmd[*]}\"" >> "$MANIFEST"
}

for name in "${MESHES[@]}"; do
  echo "== $name =="
  rm -rf "$OUT/$name"; mkdir -p "$OUT/$name"

  run_variant "$name" "agglomerate"        "-a"   "$AGG_OPTS"
  run_variant "$name" "relax"              "-r"   "$RELAX_OPTS"
  run_variant "$name" "agglomerate_relax"  "--ar" "$AR_OPTS"

  # baseline (the .off re-emitted as VTK) is identical across variants
  cp "$OUT/_runs/$name/agglomerate/vtk/input_mesh.vtk" "$OUT/$name/baseline.vtk"
done

echo
echo "Done. Per-mesh outputs under $OUT/<name>/ :"
echo "  baseline.vtk  agglomerate.vtk  relax.vtk  agglomerate_relax.vtk"
echo "Evaluate them in MATLAB with performance/matlab/evaluate_directory."
