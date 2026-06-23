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
# If run_sorgente.sh also produced the shape-metric tree output/sorgente_shape/, this
# analyzes it too, writing output/sorgente_shape/eigen.csv the same way (so the
# stability-vs-shape comparison in plot_sorgente.sh has both eigen.csv files).
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

SHAPE_OUT="$HERE/output/sorgente_shape"   # shape-metric tree (optional)

# analyze_tree <outdir> <label> <honor_meshes:yes|no>
# computes conditioning for every <name>/ folder under <outdir> and writes the
# combined <outdir>/eigen.csv. The main tree honours a non-empty $MESHES override;
# the shape tree always auto-discovers its folders.
analyze_tree() {
  local out="$1" label="$2" honor="$3"
  [[ -d "$out" ]] || die "no staged output at $out (run run_sorgente.sh first)"

  local meshes=()
  if [[ "$honor" == yes && ${#MESHES[@]} -gt 0 ]]; then
    meshes=("${MESHES[@]}")
  else
    local d
    for d in "$out"/*/; do meshes+=("$(basename "$d")"); done
  fi
  [[ ${#meshes[@]} -gt 0 ]] || die "no mesh folders found under $out"

  echo
  echo "  ${BOLD}vemesh performance · ${label} · eigen analysis${RESET}"
  rule
  printf '  %-10s %s\n' "meshes" "${#meshes[@]}"
  printf '  %-10s %s\n' "engine" "${ENGINE[*]}"
  printf '  %-10s %s\n' "output" "$out"
  rule
  echo

  # one combined CSV for every mesh/variant in this tree
  local CSV="$out/eigen.csv"
  echo "mesh,variant,lambda_2,lambda_max,ratio" > "$CSV"

  local i=0 name dir f args code rows out_rows
  local files variants
  for name in "${meshes[@]}"; do
    i=$((i + 1))
    dir="$out/$name"
    printf '  %s[%d/%d] %s%s\n' "$BOLD" "$i" "${#meshes[@]}" "$name" "$RESET"

    files=("$dir"/*.vtk)
    if [[ ${#files[@]} -eq 0 ]]; then
      printf '    %sno vtk files, skipping%s\n\n' "$DIM" "$RESET"
      continue
    fi

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

    # prepend the mesh name, strip the .vtk extension from the variant column; stray
    # engine output (e.g. a MATLAB warning) is echoed to stderr, kept out of the CSV.
    out_rows=$(printf '%s\n' "$rows" \
      | awk -v m="$name" -F, 'BEGIN{OFS=","}
          NF==4 && $1 ~ /\.vtk$/ { sub(/\.vtk$/,"",$1); print m,$1,$2,$3,$4; next }
          { print "  engine: " $0 > "/dev/stderr" }')

    printf '%s\n' "$out_rows" >> "$CSV"
    printf '%s\n' "$out_rows" | awk -F, '{
      printf "      %-18s lambda_2=%-12.4e lambda_max=%-12.4e ratio=%.4e\n", $2, $3, $4, $5
    }'
    echo
  done

  rule
  printf '  %s✓ done%s  %d mesh(es) analyzed; all eigen data in %s%s%s\n' \
         "$GREEN$BOLD" "$RESET" "${#meshes[@]}" "$CYAN" "$CSV" "$RESET"
}

analyze_tree "$OUT" "sorgente" yes
if [[ -d "$SHAPE_OUT" ]]; then
  analyze_tree "$SHAPE_OUT" "sorgente _40 (shape)" no
fi
