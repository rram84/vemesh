#!/usr/bin/env bash
# Sriramajayam
#
# Performance analysis: embedded shapes (eigen / VEM conditioning).
# For each shape folder staged by run_shapes.sh
#   output/shapes/<shape>/<realization>__<variant>.vtk
# compute the VEM stiffness conditioning (lambda_2, lambda_max, ratio) of every
# mesh and write a per-shape CSV:
#   output/shapes/<shape>.csv
# with columns: realization,variant,lambda_2,lambda_max,ratio
# (lambda_2 is the smallest NONZERO eigenvalue; the pure-Neumann stiffness has a
#  zero eigenvalue from the constant mode, so ratio = lambda_max / lambda_2.)
#
# The eigen calc is the only step that needs MATLAB/Octave. One engine call per
# shape folder pays the engine start-up cost once per shape rather than per file.

set -euo pipefail
shopt -s nullglob

# --------------------------------------------------------------------------- #
# Configuration  (edit the engine and shapes below)
# --------------------------------------------------------------------------- #

# Eigen engine: the command that runs a snippet of MATLAB/Octave code passed as a
# single string argument. Default is MATLAB; switch to Octave with:
#   ENGINE=(octave --eval)
# `matlab` must be on your PATH. On macOS the launcher lives inside the app bundle,
# so add it once in ~/.zshrc:
#   export PATH="/Applications/MATLAB_R2023a.app/bin:$PATH"
# (or point ENGINE straight at it: ENGINE=(/Applications/MATLAB_R2023a.app/bin/matlab -batch))
ENGINE=(matlab -batch)

# shapes to analyze; empty => every <shape>/ folder under $OUT. Override with stems:
#   SHAPES=(85909 260871)
SHAPES=()

# --------------------------------------------------------------------------- #
# Paths  (no edits needed; resolved relative to this script)
# --------------------------------------------------------------------------- #
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

OUT="$HERE/output/shapes"     # produced by run_shapes.sh
MATLAB_DIR="$HERE/matlab"     # symlinked in by CMake

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

command -v "${ENGINE[0]}" >/dev/null 2>&1 || die "eigen engine '${ENGINE[0]}' not found in PATH"
[[ -d "$OUT"        ]] || die "no staged output at $OUT (run run_shapes.sh first)"
[[ -d "$MATLAB_DIR" ]] || die "matlab sources not found at $MATLAB_DIR"

if [[ ${#SHAPES[@]} -eq 0 ]]; then
  for d in "$OUT"/*/; do
    b="$(basename "$d")"
    [[ "$b" == _* ]] && continue        # skip scratch / internal dirs (e.g. _scratch)
    SHAPES+=("$b")
  done
fi
[[ ${#SHAPES[@]} -gt 0 ]] || die "no shape folders found under $OUT"

# banner
echo
echo "  ${BOLD}vemesh performance · shapes · eigen analysis${RESET}"
rule
printf '  %-10s %s\n' "shapes" "${#SHAPES[@]}"
printf '  %-10s %s\n' "engine" "${ENGINE[*]}"
printf '  %-10s %s\n' "output" "$OUT/<shape>.csv"
rule
echo

i=0
for shape in "${SHAPES[@]}"; do
  i=$((i + 1))
  dir="$OUT/$shape"
  printf '  %s[%d/%d] %s%s\n' "$BOLD" "$i" "${#SHAPES[@]}" "$shape" "$RESET"

  # this shape's staged VTKs
  files=("$dir"/*.vtk)
  if [[ ${#files[@]} -eq 0 ]]; then
    printf '    %sno vtk files, skipping%s\n\n' "$DIM" "$RESET"
    continue
  fi
  printf '    %s%d meshes -> one engine call%s\n' "$DIM" "${#files[@]}" "$RESET"

  # build the engine arg list: vem_eig('f1','f2',...)
  args=""
  for f in "${files[@]}"; do args+="'$f',"; done
  args="${args%,}"

  # one engine call for the whole shape; stdout is CSV rows of
  # <realization>__<variant>.vtk,lambda_2,lambda_max,ratio; stderr to the terminal
  code="addpath('$MATLAB_DIR'); vem_eig($args);"
  rows=$("${ENGINE[@]}" "$code") \
    || die "eigen engine failed on $shape (re-run the engine on this folder to see the error)"

  # per-shape CSV: split the "<realization>__<variant>" stem into two columns
  csv="$OUT/${shape}.csv"
  {
    echo "realization,variant,lambda_2,lambda_max,ratio"
    printf '%s\n' "$rows" \
      | awk -F, 'BEGIN{OFS=","}
          NF==4 && $1 ~ /\.vtk$/ {
            stem=$1; sub(/\.vtk$/,"",stem);
            j=index(stem,"__"); realn=substr(stem,1,j-1); variant=substr(stem,j+2);
            print realn,variant,$2,$3,$4; next
          }
          { print "  engine: " $0 > "/dev/stderr" }'
  } > "$csv"

  printf '    %s→ %s%s\n\n' "$DIM" "$csv" "$RESET"
done

rule
printf '  %s✓ done%s  %d shape(s) analyzed; one CSV per shape under %s%s/%s\n' \
       "$GREEN$BOLD" "$RESET" "${#SHAPES[@]}" "$CYAN" "$OUT" "$RESET"
