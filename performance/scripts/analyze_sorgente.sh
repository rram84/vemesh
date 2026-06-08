#!/usr/bin/env bash
# Sriramajayam
#
# Performance analysis: sorgente meshes.
# For each mesh folder staged by run_sorgente.sh
#   output/sorgente/<name>/{baseline,agglomerate,relax,agglomerate_relax}.vtk
# compute the VEM stiffness conditioning (lambda_min, lambda_max, ratio) of every
# variant and write a per-mesh CSV:
#   output/sorgente/<name>/eigen.csv
#
# The eigen calc is the only step that needs MATLAB/Octave. This script owns the
# coordination (which files belong to which mesh); the engine only runs the
# per-file kernel (vem_quality, via the vem_eig wrapper), one call per mesh folder
# so the engine start-up cost is paid once per mesh rather than once per file.

set -euo pipefail
shopt -s nullglob

# --------------------------------------------------------------------------- #
# Configuration  (edit the engine and meshes below)
# --------------------------------------------------------------------------- #

# Eigen engine: the command that runs a snippet of MATLAB/Octave code passed as a
# single string argument. Default is MATLAB; switch to Octave with:
#   ENGINE=(octave --eval)
# `matlab` must be on your PATH. On macOS the launcher lives inside the app bundle,
# so add it once in ~/.zshrc:
#   export PATH="/Applications/MATLAB_R2023a.app/bin:$PATH"
# (or point ENGINE straight at it: ENGINE=(/Applications/MATLAB_R2023a.app/bin/matlab -batch))
ENGINE=(matlab -batch)

# meshes to analyze; empty => every <name>/ folder under $OUT. Override with basenames:
#   MESHES=(mesh1_20 mesh2_20 mesh3_20)
MESHES=()

# --------------------------------------------------------------------------- #
# Paths  (no edits needed; resolved relative to this script)
# --------------------------------------------------------------------------- #

# This script, the staged outputs and the matlab sources all live under
# build/performance/ (placed there by CMake). Everything resolves relative to $HERE.
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

OUT="$HERE/output/sorgente"   # produced by run_sorgente.sh
MATLAB_DIR="$HERE/matlab"     # symlinked in by CMake

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

command -v "${ENGINE[0]}" >/dev/null 2>&1 || die "eigen engine '${ENGINE[0]}' not found in PATH"
[[ -d "$OUT"        ]] || die "no staged output at $OUT (run run_sorgente.sh first)"
[[ -d "$MATLAB_DIR" ]] || die "matlab sources not found at $MATLAB_DIR"

if [[ ${#MESHES[@]} -eq 0 ]]; then
  for d in "$OUT"/*/; do MESHES+=("$(basename "$d")"); done
fi
[[ ${#MESHES[@]} -gt 0 ]] || die "no mesh folders found under $OUT"

# banner
echo
echo "  ${BOLD}vemesh performance · sorgente · eigen analysis${RESET}"
rule
printf '  %-10s %s\n' "meshes" "${#MESHES[@]}"
printf '  %-10s %s\n' "engine" "${ENGINE[*]}"
printf '  %-10s %s\n' "output" "$OUT"
rule
echo

i=0
for name in "${MESHES[@]}"; do
  i=$((i + 1))
  dir="$OUT/$name"
  printf '  %s[%d/%d] %s%s\n' "$BOLD" "$i" "${#MESHES[@]}" "$name" "$RESET"

  # this mesh's variant VTKs (eigen.csv from a prior run is ignored: not *.vtk)
  files=("$dir"/*.vtk)
  if [[ ${#files[@]} -eq 0 ]]; then
    printf '    %sno vtk files, skipping%s\n\n' "$DIM" "$RESET"
    continue
  fi

  # build the engine arg list: vem_eig('f1','f2',...)
  args=""
  for f in "${files[@]}"; do args+="'$f',"; done
  args="${args%,}"

  # one engine call for the whole folder; stdout is the CSV body, stderr (engine
  # warnings/errors) flows to the terminal so failures are visible
  code="addpath('$MATLAB_DIR'); vem_eig($args);"
  rows=$("${ENGINE[@]}" "$code") \
    || die "eigen engine failed on $name (re-run the engine on this folder to see the error)"

  # per-mesh CSV
  csv="$dir/eigen.csv"
  { echo "name,lambda_min,lambda_max,ratio"; printf '%s\n' "$rows"; } > "$csv"

  printf '%s\n' "$rows" | sed 's/^/      /'
  printf '    %s→ %s%s\n\n' "$DIM" "$csv" "$RESET"
done

rule
printf '  %s✓ done%s  %d mesh(es) analyzed; eigen.csv written under %s%s/<name>/%s\n' \
       "$GREEN$BOLD" "$RESET" "${#MESHES[@]}" "$CYAN" "$OUT" "$RESET"
