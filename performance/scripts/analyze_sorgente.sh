#!/usr/bin/env bash
# Sriramajayam
#
# Performance analysis: sorgente meshes.
# For each mesh folder staged by run_sorgente.sh
#   output/sorgente/<name>/{baseline,agglomerate,relax,agglomerate_relax}.vtk
# compute the VEM stiffness conditioning (lambda_2, lambda_max, ratio) of every
# variant and collect them all into one combined CSV:
#   output/sorgente/eigen.csv
# with columns: mesh,variant,lambda_2,lambda_max,ratio
# (lambda_2 is the smallest NONZERO eigenvalue; the pure-Neumann stiffness has a
#  zero eigenvalue from the constant mode, so ratio = lambda_max / lambda_2.)
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

# one combined CSV for every mesh/variant
CSV="$OUT/eigen.csv"
echo "mesh,variant,lambda_2,lambda_max,ratio" > "$CSV"

i=0
for name in "${MESHES[@]}"; do
  i=$((i + 1))
  dir="$OUT/$name"
  printf '  %s[%d/%d] %s%s\n' "$BOLD" "$i" "${#MESHES[@]}" "$name" "$RESET"

  # this mesh's variant VTKs
  files=("$dir"/*.vtk)
  if [[ ${#files[@]} -eq 0 ]]; then
    printf '    %sno vtk files, skipping%s\n\n' "$DIM" "$RESET"
    continue
  fi

  # detail on what is being analyzed for this mesh
  variants=()
  for f in "${files[@]}"; do variants+=("$(basename "$f" .vtk)"); done
  printf '    %s%-9s%s %s\n' "$DIM" "folder"   "$RESET" "$dir"
  printf '    %s%-9s%s %d (%s)\n' "$DIM" "variants" "$RESET" "${#files[@]}" "${variants[*]}"

  # build the engine arg list: vem_eig('f1','f2',...)
  args=""
  for f in "${files[@]}"; do args+="'$f',"; done
  args="${args%,}"

  # one engine call for the whole folder; stdout is CSV rows of
  # <variant>.vtk,lambda_2,lambda_max,ratio; stderr flows to the terminal
  code="addpath('$MATLAB_DIR'); vem_eig($args);"
  rows=$("${ENGINE[@]}" "$code") \
    || die "eigen engine failed on $name (re-run the engine on this folder to see the error)"

  # prepend the mesh name and strip the .vtk extension from the variant column
  out=$(printf '%s\n' "$rows" \
    | awk -v m="$name" -F, 'BEGIN{OFS=","} {sub(/\.vtk$/,"",$1); print m,$1,$2,$3,$4}')

  # append the raw rows to the single combined CSV
  printf '%s\n' "$out" >> "$CSV"

  # echo a labeled view of the same numbers to the terminal (scientific notation)
  printf '%s\n' "$out" | awk -F, '{
    printf "      %-18s lambda_2=%-12.4e lambda_max=%-12.4e ratio=%.4e\n", $2, $3, $4, $5
  }'
  echo
done

rule
printf '  %s✓ done%s  %d mesh(es) analyzed; all eigen data in %s%s%s\n' \
       "$GREEN$BOLD" "$RESET" "${#MESHES[@]}" "$CYAN" "$CSV" "$RESET"
