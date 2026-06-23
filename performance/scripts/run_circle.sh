#!/usr/bin/env bash
# Sriramajayam
#
# Performance test: clipped circle — mesh generation + improvement.
#
# For each background-mesh refinement level, generate N randomly-perturbed
# clippings of a structured quad mesh to a circular disk (clip_circle -d <level>)
# and improve each with vemesh_app. Every resulting mesh is KEPT on disk; the
# VEM-conditioning (eigen) analysis is a separate step — run analyze_circle.sh afterwards.
#
# The refinement level plays the same role here that the interface polygon plays
# in run_shapes.sh: one output folder per level, named after the level.
#
# Output layout (one folder per level; meshes are not erased):
#   output/circle/<level>/<realization>__baseline.vtk
#   output/circle/<level>/<realization>__agglomerate.vtk
#   output/circle/<level>/<realization>__relax.vtk
#   output/circle/<level>/<realization>__agglomerate_relax.vtk
# baseline = the unimproved clipped mesh; the others are vemesh_app variants.

set -euo pipefail
shopt -s nullglob

# --------------------------------------------------------------------------- #
# Configuration  (edit levels, counts and algorithm options below)
# --------------------------------------------------------------------------- #

# background-mesh refinement levels to sweep (clip_circle -d). Level 0 is h=0.2 /
# ncount=10; each level halves h and doubles ncount. One output folder per level.
LEVELS=(
  0
  1
  2
  3
)

# clip_circle options (everything except -o, -d and -S, which the script supplies).
# -n sets the number of randomly-perturbed realizations generated per level.
CLIP_OPTS="-n 3"

# base RNG seed for clip_circle (one seed for the whole set, so it is reproducible)
SEED=12345

# vemesh_app options per variant (everything except -i/-o and the mode flag).
# -S <seed> is included on the relaxation variants for reproducibility.
AGG_OPTS="-n 6 -q 0.2 -f 1.2 -m stability"
RELAX_OPTS="-n 6 -q 0.2 -s 20 -m stability -S 12345"
AR_OPTS="-n 3 -q 0.2 -f 1.2 -s 20 -m stability -S 12345"

# variants run on each clipped mesh, as "name|<mode-flag>|<opts>" (an indexed
# array, so it works on the stock macOS bash 3.2 which lacks associative arrays).
# the baseline (the unimproved clipped mesh) is always staged in addition.
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

VEMESH_APP="$HERE/vemesh_app"                  # built alongside this script
CLIP="$HERE/mesh_generators/clip_circle"       # mesh generator (self-contained)
OUT="$HERE/output/circle"

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
[[ -x "$CLIP"       ]] || die "clip_circle not found at $CLIP (build with -DBUILD_TESTS=ON)"
[[ ${#LEVELS[@]} -gt 0 ]] || die "no refinement levels configured"

mkdir -p "$OUT"

# variant names (for the banner), extracted from the "name|mode|opts" entries
variant_names="baseline"
for entry in "${VARIANTS[@]}"; do variant_names+=" ${entry%%|*}"; done

# banner
echo
echo "  ${BOLD}vemesh performance · circle · generate + improve${RESET}"
rule
printf '  %-18s %s\n' "domain"             "disk (clipped to circle)"
printf '  %-18s %s\n' "subdivision levels" "${LEVELS[*]}"
printf '  %-18s %s\n' "clip opts"          "$CLIP_OPTS"
printf '  %-18s %s\n' "variants"           "$variant_names"
printf '  %-18s %s\n' "output"             "$OUT/<level>/"
rule
echo

ci=0
for level in "${LEVELS[@]}"; do
  ci=$((ci + 1))

  leveldir="$OUT/$level"
  rm -rf "$leveldir"; mkdir -p "$leveldir"      # fresh output folder for this run

  # per-level scratch directory, separate from the kept meshes, so running the
  # levels one at a time — or side by side in different terminals — never shares
  # scratch. Removed when this level finishes; the staged meshes under $leveldir remain.
  scratch="$OUT/_scratch/$level"
  rm -rf "$scratch"; mkdir -p "$scratch/clip"

  printf '  %s[%d/%d] subdivision level %s%s\n' "$BOLD" "$ci" "${#LEVELS[@]}" "$level" "$RESET"

  # 1) generate all clipped meshes for this level in one go -> scratch/clip/clip-<i>.vtk
  #    the realization count lives in $CLIP_OPTS (-n); the script supplies -o/-d/-S.
  clipdir="$scratch/clip"
  printf '    %sgenerating clipped meshes (seed %d)...%s\n' "$DIM" "$SEED" "$RESET"
  if ! "$CLIP" -o "$clipdir" -d "$level" -S "$SEED" $CLIP_OPTS \
                > "$leveldir/clip.log" 2>&1; then
    die "clip_circle failed for level $level (see $leveldir/clip.log)"
  fi

  # 2a) the clipped meshes are the variant INPUTS, not staged as-is: the baseline is
  #     captured from vemesh_app's input_mesh.vtk during the first variant run below,
  #     so it carries face_quality like the improved variants (not the raw generator
  #     output, which has none). glob so the count comes from $CLIP_OPTS (-n).
  inputs=("$clipdir"/clip-*.vtk)
  [[ ${#inputs[@]} -gt 0 ]] || die "clip_circle produced no meshes (see $leveldir/clip.log)"
  printf '    %s%d realizations to improve...%s\n' "$DIM" "${#inputs[@]}" "$RESET"

  # 2b) improve variant-by-variant: each variant runs across every realization before
  #     moving on to the next (agglomerate, then agglomerate+relax, then relax).
  #     vemesh_app writes input_mesh.vtk (the input mesh WITH face_quality) before
  #     optimizing; the FIRST variant pass stages that as the baseline, so baseline
  #     carries quality -- exactly as run_sorgente.sh does.
  run="$scratch/run"                            # vemesh_app scratch (intermediates)
  first=1
  for entry in "${VARIANTS[@]}"; do
    IFS='|' read -r name mode opts <<< "$entry"
    printf '    %s%s for all %d realizations...%s\n' "$DIM" "$name" "${#inputs[@]}" "$RESET"
    n=0
    for inp in "${inputs[@]}"; do
      n=$((n + 1))
      i="$(basename "$inp" .vtk)"; i="${i#clip-}"   # clip-<i>.vtk -> <i>
      rm -rf "$run"; mkdir -p "$run"
      if ! out=$("$VEMESH_APP" "$mode" -i "$inp" -o "$run" $opts 2>&1); then
        printf '%s\n' "$out" | sed 's/^/      | /'
        die "vemesh_app $mode failed on level $level realization $i"
      fi
      cp "$run/output_mesh.vtk" "$leveldir/${i}__${name}.vtk"
      if (( first )); then cp "$run/input_mesh.vtk" "$leveldir/${i}__baseline.vtk"; fi
      if (( n % PROGRESS_EVERY == 0 || n == ${#inputs[@]} )); then
        printf '      %s%s %d/%d%s\n' "$DIM" "$name" "$n" "${#inputs[@]}" "$RESET"
      fi
    done
    first=0
  done

  # drop this level's scratch entirely; the staged meshes under $leveldir remain
  rm -rf "$scratch"
  echo
done

rule
printf '  %s✓ done%s  %d subdivision level(s) staged under %s%s/<level>/%s\n' \
       "$GREEN$BOLD" "$RESET" "${#LEVELS[@]}" "$CYAN" "$OUT" "$RESET"
echo "    <realization>__{baseline,agglomerate,relax,agglomerate_relax}.vtk"
echo "  ${DIM}compute VEM conditioning: ./analyze_circle.sh${RESET}"
