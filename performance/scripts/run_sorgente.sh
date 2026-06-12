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
#
# It ALSO runs a shape-metric companion pass on the _40 meshes (mesh*_40) into a
# separate tree performance/output/sorgente_shape/, so the VEM-stability vs geometric
# -shape comparison (plot_sorgente.sh make_compare_figure) is reproducible from this
# one script. The shape option strings are the stability ones with -m shape.

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
AGG_OPTS="-n 6 -q 0.2 -f 1.2 -m stability"                  # agglomeration only            (-a)
RELAX_OPTS="-n 6 -q 0.2 -s 20 -m stability -S 12345"        # relaxation only               (-r)
# --ar does an agglomerate AND a relax per iteration, so -n 3 == 6 operations,
# a fair comparison with the 6 single-operation iterations above.
AR_OPTS="-n 3 -q 0.2 -f 1.2 -s 20 -m stability -S 12345"    # alternate agglomerate + relax (--ar)

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

# colours (only when writing to a terminal)
if [[ -t 1 ]]; then
  BOLD=$'\e[1m'; DIM=$'\e[2m'; GREEN=$'\e[32m'; RED=$'\e[31m'; CYAN=$'\e[36m'; RESET=$'\e[0m'
else
  BOLD=''; DIM=''; GREEN=''; RED=''; CYAN=''; RESET=''
fi
rule() { printf '%s\n' "  ────────────────────────────────────────────────────────"; }
die()  { printf '%s\n' "${RED}error:${RESET} $*" >&2; exit 1; }

[[ -x "$VEMESH_APP" ]] || die "vemesh_app not found at $VEMESH_APP (build with -DBUILD_TESTS=ON)"
[[ -d "$SORGENTE"   ]] || die "sorgente dir not found at $SORGENTE"

if [[ ${#MESHES[@]} -eq 0 ]]; then
  for f in "$SORGENTE"/*.off; do MESHES+=("$(basename "$f" .off)"); done
fi
[[ ${#MESHES[@]} -gt 0 ]] || die "no .off meshes found in $SORGENTE"

mkdir -p "$OUT"

# banner
echo
echo "  ${BOLD}vemesh performance · sorgente${RESET}"
rule
printf '  %-10s %s\n' "meshes" "${#MESHES[@]}"
printf '  %-10s %s\n' "output" "$OUT"
echo
printf '  %-12s %s\n' "agglomerate"  "$DIM$AGG_OPTS$RESET"
printf '  %-12s %s\n' "relax"        "$DIM$RELAX_OPTS$RESET"
printf '  %-12s %s\n' "agg+relax"    "$DIM$AR_OPTS$RESET"
rule
echo

# run_variant <name> <variant> <mode-flag> <opts> <outbase>
# runs vemesh_app quietly into <outbase>/<name>/; one status line, dump on failure.
run_variant() {
  local name="$1" variant="$2" mode="$3" opts="$4" outbase="$5"
  local run="$outbase/_runs/$name"      # scratch dir, wiped fresh for each run
  rm -rf "$run"; mkdir -p "$run"

  printf '    %-18s' "$variant"
  local t0=$SECONDS out
  if out=$("$VEMESH_APP" "$mode" -i "$SORGENTE/$name.off" -o "$run" $opts 2>&1); then
    cp "$run/output_mesh.vtk"         "$outbase/$name/$variant.vtk"
    cp "$run/output_mesh_quality.dat" "$outbase/$name/${variant}_quality.dat"
    printf ' %s✓%s %s(%ds)%s\n' "$GREEN" "$RESET" "$DIM" "$((SECONDS - t0))" "$RESET"
  else
    printf ' %s✗%s\n' "$RED" "$RESET"
    printf '%s\n' "$out" | sed 's/^/      | /'
    die "$variant failed on $name"
  fi
}

# process_mesh <name> <outbase> <agg-opts> <relax-opts> <ar-opts>
# stages baseline (from agglomerate's input_mesh, which carries face_quality) plus
# the three improved variants into <outbase>/<name>/.
process_mesh() {
  local name="$1" outbase="$2" agg="$3" relax="$4" ar="$5"
  rm -rf "$outbase/$name" "$outbase/_runs/$name"; mkdir -p "$outbase/$name"
  run_variant "$name" "agglomerate"       "-a"   "$agg"   "$outbase"
  # input mesh (re-emitted as VTK); identical across variants, stage it now so the
  # baseline is present as soon as the first variant completes.
  cp "$outbase/_runs/$name/input_mesh.vtk"         "$outbase/$name/baseline.vtk"
  cp "$outbase/_runs/$name/input_mesh_quality.dat" "$outbase/$name/baseline_quality.dat"
  run_variant "$name" "relax"             "-r"   "$relax" "$outbase"
  run_variant "$name" "agglomerate_relax" "--ar" "$ar"    "$outbase"
}

i=0
for name in "${MESHES[@]}"; do
  i=$((i + 1))
  printf '  %s[%d/%d] %s%s\n' "$BOLD" "$i" "${#MESHES[@]}" "$name" "$RESET"
  process_mesh "$name" "$OUT" "$AGG_OPTS" "$RELAX_OPTS" "$AR_OPTS"
  echo
done
rm -rf "$OUT/_runs"      # discard scratch; the curated <name>/ folders are enough

# --------------------------------------------------------------------------- #
# Shape-metric companion pass: the SAME _40 meshes optimised for the geometric
# SHAPE metric (instead of VEM stability), into a SEPARATE output tree so the two
# metrics can be compared. Option strings are the stability ones with the metric
# swapped (-m stability -> -m shape), so the two stay in sync automatically.
# --------------------------------------------------------------------------- #
SHAPE_OUT="$HERE/output/sorgente_shape"
SHAPE_MESHES=()
for name in "${MESHES[@]}"; do
  if [[ "$name" == *_40 ]]; then SHAPE_MESHES+=("$name"); fi
done

if (( ${#SHAPE_MESHES[@]} > 0 )); then
  mkdir -p "$SHAPE_OUT"
  echo
  rule
  printf '  %sshape-metric pass · %d _40 mesh(es) -> %s%s\n' \
         "$BOLD" "${#SHAPE_MESHES[@]}" "$SHAPE_OUT" "$RESET"
  rule
  echo
  SH_AGG="${AGG_OPTS/-m stability/-m shape}"
  SH_RELAX="${RELAX_OPTS/-m stability/-m shape}"
  SH_AR="${AR_OPTS/-m stability/-m shape}"
  i=0
  for name in "${SHAPE_MESHES[@]}"; do
    i=$((i + 1))
    printf '  %s[%d/%d] %s %s(shape)%s\n' "$BOLD" "$i" "${#SHAPE_MESHES[@]}" "$name" "$DIM" "$RESET"
    process_mesh "$name" "$SHAPE_OUT" "$SH_AGG" "$SH_RELAX" "$SH_AR"
    echo
  done
  rm -rf "$SHAPE_OUT/_runs"
fi

rule
printf '  %s✓ done%s  %d mesh(es) staged under %s%s/<name>/%s\n' \
       "$GREEN$BOLD" "$RESET" "${#MESHES[@]}" "$CYAN" "$OUT" "$RESET"
if (( ${#SHAPE_MESHES[@]} > 0 )); then
  printf '  %s        %s  %d _40 mesh(es) (shape metric) under %s%s/<name>/%s\n' \
         "$GREEN$BOLD" "$RESET" "${#SHAPE_MESHES[@]}" "$CYAN" "$SHAPE_OUT" "$RESET"
fi
echo "    baseline.vtk  agglomerate.vtk  relax.vtk  agglomerate_relax.vtk"
echo "  ${DIM}compute VEM conditioning: ./analyze_sorgente.sh${RESET}"
