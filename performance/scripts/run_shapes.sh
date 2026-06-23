#!/usr/bin/env bash
# Sriramajayam
#
# Performance test: embedded shapes — mesh generation + improvement.
#
# For each interface polygon (on a fixed background mesh), generate N randomly
# perturbed embeddings (embed_shapes) and improve each with vemesh_app. Every
# resulting mesh is KEPT on disk; the VEM-conditioning (eigen) analysis is a
# separate step — run analyze_shapes.sh afterwards.
#
# Output layout (one folder per shape; meshes are not erased):
#   output/shapes/<shape>/<realization>__baseline.vtk
#   output/shapes/<shape>/<realization>__agglomerate.vtk
#   output/shapes/<shape>/<realization>__relax.vtk
#   output/shapes/<shape>/<realization>__agglomerate_relax.vtk
# baseline = the unimproved embedded mesh; the others are vemesh_app variants.

set -euo pipefail
shopt -s nullglob

# --------------------------------------------------------------------------- #
# Configuration  (edit shapes, counts and algorithm options below)
# --------------------------------------------------------------------------- #

# Background triangle mesh, held fixed for every shape (under sample_data/tri).
BACKGROUND="bbbb-3.off"

# Interface polygons to sweep (under sample_data/shapes); one output folder per
# shape, named after the shape's file stem (e.g. 85909.dat -> output/shapes/85909/).
# list of shapes:  85909.dat 260871.dat 202418.dat 17333.dat 235004.dat
SHAPES=(
  85909.dat
  260871.dat
  202418.dat
  17333.dat
  235004.dat
)

# number of randomly-perturbed realizations per shape (all generated in one go)
N_REALIZATIONS=3

# base RNG seed for embed_shapes (one seed for the whole set, so it is reproducible)
SEED=12345

# vemesh_app options per variant (everything except -i/-o and the mode flag).
# -S <seed> is included on the relaxation variants for reproducibility.
AGG_OPTS="-n 6 -q 0.2 -f 1.2 -m stability"
RELAX_OPTS="-n 6 -q 0.2 -s 20 -m stability -S 12345"
AR_OPTS="-n 3 -q 0.2 -f 1.2 -s 20 -m stability -S 12345"

# variants run on each embedded mesh, as "name|<mode-flag>|<opts>" (an indexed
# array, so it works on the stock macOS bash 3.2 which lacks associative arrays).
# the baseline (the unimproved embedded mesh) is always staged in addition.
VARIANTS=(
  "agglomerate|-a|$AGG_OPTS"
  "agglomerate_relax|--ar|$AR_OPTS"
  "relax|-r|$RELAX_OPTS"
)

# print a progress line every this many realizations
PROGRESS_EVERY=20

# --------------------------------------------------------------------------- #
# Paths  (no edits needed; resolved relative to this script)
# --------------------------------------------------------------------------- #
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

VEMESH_APP="$HERE/vemesh_app"                 # built alongside this script
EMBED="$HERE/mesh_generators/embed_shapes"    # mesh generator
SHAPES_DIR="$HERE/sample_data/shapes"         # interface polygons (.dat)
TRI_DIR="$HERE/sample_data/tri"               # background triangle meshes (.off)
OUT="$HERE/output/shapes"

# --------------------------------------------------------------------------- #
# Driver  (no edits needed below)
# --------------------------------------------------------------------------- #
if [[ -t 1 ]]; then
  BOLD=$'\e[1m'; DIM=$'\e[2m'; GREEN=$'\e[32m'; RED=$'\e[31m'; CYAN=$'\e[36m'; RESET=$'\e[0m'
else
  BOLD=''; DIM=''; GREEN=''; RED=''; CYAN=''; RESET=''
fi
rule() { printf '%s\n' "  ────────────────────────────────────────────────────────"; }
die()  { printf '%s\n' "${RED}error:${RESET} $*" >&2; exit 1; }

[[ -x "$VEMESH_APP" ]] || die "vemesh_app not found at $VEMESH_APP (build with -DBUILD_TESTS=ON)"
[[ -x "$EMBED"      ]] || die "embed_shapes not found at $EMBED (build with -DBUILD_TESTS=ON)"
[[ -d "$SHAPES_DIR" ]] || die "shapes dir not found at $SHAPES_DIR"
[[ ${#SHAPES[@]} -gt 0 ]] || die "no shapes configured"

bg_path="$TRI_DIR/$BACKGROUND"
[[ -f "$bg_path" ]] || die "background mesh not found: $bg_path"

mkdir -p "$OUT"

# variant names (for the banner), extracted from the "name|mode|opts" entries
variant_names="baseline"
for entry in "${VARIANTS[@]}"; do variant_names+=" ${entry%%|*}"; done

# banner
echo
echo "  ${BOLD}vemesh performance · shapes · generate + improve${RESET}"
rule
printf '  %-12s %s\n' "shapes"       "${#SHAPES[@]}"
printf '  %-12s %s\n' "background"   "$BACKGROUND"
printf '  %-12s %s\n' "realizations" "$N_REALIZATIONS"
printf '  %-12s %s\n' "variants"     "$variant_names"
printf '  %-12s %s\n' "output"       "$OUT/<shape>/"
rule
echo

ci=0
for shape in "${SHAPES[@]}"; do
  ci=$((ci + 1))
  label="${shape%.*}"                 # 85909.dat -> 85909
  geom_path="$SHAPES_DIR/$shape"
  [[ -f "$geom_path" ]] || die "interface file not found: $geom_path"

  shapedir="$OUT/$label"
  rm -rf "$shapedir"; mkdir -p "$shapedir"      # fresh output folder for this run

  # per-shape scratch directory, separate from the kept meshes, so running the
  # shapes one at a time — or side by side in different terminals — never shares
  # scratch. Removed when this shape finishes; the staged meshes under $shapedir remain.
  scratch="$OUT/_scratch/$label"
  rm -rf "$scratch"; mkdir -p "$scratch/embed"

  printf '  %s[%d/%d] %s%s  %s(%s on %s)%s\n' \
         "$BOLD" "$ci" "${#SHAPES[@]}" "$label" "$RESET" "$DIM" "$shape" "$BACKGROUND" "$RESET"

  # 1) generate all embedded meshes in one go -> scratch/embed/embed-<i>.vtk
  embeddir="$scratch/embed"
  printf '    %sgenerating %d embedded meshes (seed %d)...%s\n' "$DIM" "$N_REALIZATIONS" "$SEED" "$RESET"
  if ! "$EMBED" -g "$geom_path" -i "$bg_path" -o "$embeddir" \
                -n "$N_REALIZATIONS" -S "$SEED" > "$shapedir/embed.log" 2>&1; then
    die "embed_shapes failed for $label (see $shapedir/embed.log)"
  fi

  # 2a) the embedded meshes are the variant INPUTS, not staged as-is: the baseline is
  #     captured from vemesh_app's input_mesh.vtk during the first variant run below,
  #     so it carries face_quality like the improved variants (not the raw generator
  #     output, which has none).
  for (( i=0; i<N_REALIZATIONS; i++ )); do
    [[ -f "$embeddir/embed-$i.vtk" ]] || die "expected embedded mesh missing: $embeddir/embed-$i.vtk"
  done

  # 2b) improve variant-by-variant: each variant runs across every realization before
  #     moving on to the next (agglomerate, then agglomerate+relax, then relax).
  #     vemesh_app writes input_mesh.vtk (the input mesh WITH face_quality) before
  #     optimizing; the FIRST variant pass stages that as the baseline, so baseline
  #     carries quality -- exactly as run_sorgente.sh does.
  run="$scratch/run"                            # vemesh_app scratch (intermediates)
  first=1
  for entry in "${VARIANTS[@]}"; do
    IFS='|' read -r name mode opts <<< "$entry"
    printf '    %s%s for all %d realizations...%s\n' "$DIM" "$name" "$N_REALIZATIONS" "$RESET"
    for (( i=0; i<N_REALIZATIONS; i++ )); do
      emb="$embeddir/embed-$i.vtk"
      rm -rf "$run"; mkdir -p "$run"
      if ! out=$("$VEMESH_APP" "$mode" -i "$emb" -o "$run" $opts 2>&1); then
        printf '%s\n' "$out" | sed 's/^/      | /'
        die "vemesh_app $mode failed on $label realization $i"
      fi
      cp "$run/output_mesh.vtk" "$shapedir/${i}__${name}.vtk"
      if (( first )); then cp "$run/input_mesh.vtk" "$shapedir/${i}__baseline.vtk"; fi
      if (( (i + 1) % PROGRESS_EVERY == 0 || i + 1 == N_REALIZATIONS )); then
        printf '      %s%s %d/%d%s\n' "$DIM" "$name" "$(( i + 1 ))" "$N_REALIZATIONS" "$RESET"
      fi
    done
    first=0
  done

  # drop this shape's scratch entirely; the staged meshes under $shapedir remain
  rm -rf "$scratch"
  echo
done

rule
printf '  %s✓ done%s  %d shape(s) staged under %s%s/<shape>/%s\n' \
       "$GREEN$BOLD" "$RESET" "${#SHAPES[@]}" "$CYAN" "$OUT" "$RESET"
echo "    <realization>__{baseline,agglomerate,relax,agglomerate_relax}.vtk"
echo "  ${DIM}compute VEM conditioning: ./analyze_shapes.sh${RESET}"
