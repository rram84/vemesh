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

# parallelism is OVER SHAPES: each shape is one MATLAB/Octave engine, and up to
# $JOBS shapes run at once. Peak memory ~ JOBS x (one engine, ~1-2 GB for MATLAB),
# so lower this if you are memory-bound. Default = CPU count (but never more than
# the number of shapes, since that is the unit of parallelism).
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

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

# cap JOBS at the number of shapes (the unit of parallelism)
(( JOBS > ${#SHAPES[@]} )) && JOBS=${#SHAPES[@]}
(( JOBS < 1 )) && JOBS=1

# banner
echo
echo "  ${BOLD}vemesh performance · shapes · eigen analysis${RESET}"
rule
printf '  %-10s %s\n' "shapes" "${#SHAPES[@]}"
printf '  %-10s %s\n' "engine" "${ENGINE[*]}"
printf '  %-10s %s (parallel over shapes)\n' "jobs" "$JOBS"
printf '  %-10s %s\n' "output" "$OUT/<shape>.csv"
rule
echo

# ------------------------------------------------------------------------- #
# Analyze ONE shape end to end: one engine call over all its meshes, parse to
# the per-shape CSV. Runs as a background job (one per shape); every line it
# prints is prefixed with the shape so the interleaved parallel output stays
# readable. Returns non-zero on engine failure so the parent's wait() catches it.
# ------------------------------------------------------------------------- #
analyze_one_shape() {
  local shape="$1" dir="$OUT/$shape"
  local t0=$SECONDS

  local files=("$dir"/*.vtk)
  local n=${#files[@]}
  if (( n == 0 )); then
    printf '  %s[%s]%s no vtk files, skipping\n' "$BOLD" "$shape" "$RESET"
    return 0
  fi

  # quick breakdown for the start line: realizations = # baseline meshes,
  # and the set of variants present (globbed; nullglob keeps empties safe)
  local variants f b
  local bfiles=("$dir"/*__baseline.vtk)
  local nbase=${#bfiles[@]}
  variants=$(for f in "$dir"/0__*.vtk; do b=$(basename "$f" .vtk); echo "${b#0__}"; done | sort -u | paste -sd, -)
  printf '  %s[%s]%s start: %d meshes  (%s realizations × {%s})\n' \
         "$BOLD" "$shape" "$RESET" "$n" "$nbase" "$variants"

  # build the engine arg list: vem_eig('f1','f2',...)
  local args="" f
  for f in "${files[@]}"; do args+="'$f',"; done
  args="${args%,}"

  # one engine call for the whole shape, STREAMED: vem_eig prints one CSV line per
  # mesh, so a single awk in the pipe parses each line straight into the per-shape
  # CSV *as it arrives* -- header written up front, each row appended and flushed
  # per mesh -- and prints a "[shape] k/n" tally to stderr every 50 meshes (and on
  # the last one) to keep the terminal readable. The CSV thus
  # exists from the start of the shape and grows live (rather than only at the end
  # of the whole call). MATLAB block-buffers its pipe stdout, so both the file and
  # the tally advance in bursts. Stray engine lines (e.g. warnings) are echoed
  # (prefixed) and kept out of the CSV.
  local csv="$OUT/${shape}.csv"
  echo "realization,variant,lambda_2,lambda_max,ratio" > "$csv"
  if ! "${ENGINE[@]}" "addpath('$MATLAB_DIR'); vem_eig($args);" \
       | awk -F, -v sh="$shape" -v tot="$n" -v csv="$csv" 'BEGIN{OFS=","}
           NF==4 && $1 ~ /\.vtk$/ {
             stem=$1; sub(/\.vtk$/,"",stem);
             j=index(stem,"__"); realn=substr(stem,1,j-1); variant=substr(stem,j+2);
             print realn,variant,$2,$3,$4 >> csv;
             c++; if (c % 50 == 0 || c == tot) { printf "  [%s] %d/%d\n", sh, c, tot > "/dev/stderr" }
             fflush(); next
           }
           { print "  [" sh "] engine: " $0 > "/dev/stderr" }'; then
    printf '  %s[%s]%s %sengine FAILED%s after %ds (partial %s kept)\n' \
           "$BOLD" "$shape" "$RESET" "$RED" "$RESET" "$((SECONDS - t0))" "$csv" >&2
    return 1
  fi

  # done line: row count + a quick conditioning summary (min/median/max ratio).
  # sort the ratio column (portable, no gawk asort) then index for the summary.
  local nrows rmin rmed rmax
  nrows=$(($(wc -l < "$csv") - 1))
  read -r rmin rmed rmax < <(awk -F, 'NR>1{print $5}' "$csv" | sort -g \
      | awk '{v[NR]=$1} END{ if(NR==0){print "- - -"; exit}
              printf "%.3g %.3g %.3g\n", v[1], v[int((NR+1)/2)], v[NR] }') \
    || { rmin=-; rmed=-; rmax=-; }
  printf '  %s[%s]%s done: %d rows in %ds  (ratio min/med/max = %s / %s / %s)  → %s\n' \
         "$GREEN$BOLD" "$shape" "$RESET" "$nrows" "$((SECONDS - t0))" "$rmin" "$rmed" "$rmax" "$csv"
}

# ------------------------------------------------------------------------- #
# Driver: process shapes in batches of $JOBS, each shape a background engine.
# A batch barrier (wait) bounds concurrency to $JOBS, so peak memory is capped.
# ------------------------------------------------------------------------- #
overall_t0=$SECONDS
nfail=0
idx=0
ns=${#SHAPES[@]}
while (( idx < ns )); do
  pids=(); batch=()
  for (( j=0; j<JOBS && idx<ns; j++, idx++ )); do
    shape="${SHAPES[$idx]}"
    analyze_one_shape "$shape" &
    pids+=("$!"); batch+=("$shape")
  done
  printf '  %s── batch of %d running ──%s\n' "$DIM" "${#pids[@]}" "$RESET"
  for k in "${!pids[@]}"; do
    if ! wait "${pids[$k]}"; then
      printf '  %sshape %s failed%s\n' "$RED" "${batch[$k]}" "$RESET" >&2
      nfail=$((nfail + 1))
    fi
  done
done

rule
if (( nfail > 0 )); then
  die "$nfail of $ns shape(s) failed; see the engine output above"
fi
printf '  %s✓ done%s  %d shape(s) analyzed in %ds; one CSV per shape under %s%s/%s\n' \
       "$GREEN$BOLD" "$RESET" "$ns" "$((SECONDS - overall_t0))" "$CYAN" "$OUT" "$RESET"
