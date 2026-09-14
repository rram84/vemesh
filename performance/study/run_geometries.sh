#!/usr/bin/env bash
# Sriramajayam
#
# Run the relaxation-vs-agglomeration study over MANY geometries, delegating each
# to run_geometry.sh (which parallelizes over realizations internally). Prints the
# geometry index as it goes, and is RESUMABLE: a geometry whose output already has
# the expected number of rows is skipped, so re-running continues where it left off.
#
# Usage:  ./run_geometries.sh [BG] [N_REAL] [SEED] [GEOM ...]
#   BG      background: quad | tri            (default quad)
#   N_REAL  realizations per geometry         (default 250)
#   SEED    base RNG seed                     (default 12345)
#   GEOM... explicit geometry stems to run    (default: ALL in dataset/geometries)
#
# Examples:
#   ./run_geometries.sh quad 250                 # all geometries, quad, 250 real
#   ./run_geometries.sh quad 250 12345 85909 100207   # just these two
#   JOBS=12 ./run_geometries.sh quad 250         # cap the per-geometry worker pool

set -euo pipefail
shopt -s nullglob

BG="${1:-quad}"
N_REAL="${2:-250}"
SEED="${3:-12345}"
shift 3 2>/dev/null || true
EXPLICIT=("$@")               # optional explicit geometry list

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUN_ONE="$HERE/run_geometry.sh"
GEO_DIR="$HERE/dataset/geometries"
OUT_DIR="$HERE/output"

die() { printf 'error: %s\n' "$*" >&2; exit 1; }
[[ -x "$RUN_ONE" ]] || die "run_geometry.sh not found/executable at $RUN_ONE"
[[ -d "$GEO_DIR" ]] || die "geometry dir not found: $GEO_DIR"

# geometry list: explicit args, else every stem in the dataset (sorted)
GEOMS=()
if (( ${#EXPLICIT[@]} > 0 )); then
  GEOMS=("${EXPLICIT[@]}")
else
  for f in "$GEO_DIR"/*.dat; do GEOMS+=("$(basename "$f" .dat)"); done
fi
IFS=$'\n' GEOMS=($(sort -n <<<"${GEOMS[*]}")); unset IFS
N=${#GEOMS[@]}
(( N > 0 )) || die "no geometries to run"

expected=$(( N_REAL * 49 ))   # meshes per realization: 1 baseline + 2 drivers x 4 workflows x 6 ops

echo
echo "  study sweep  ·  background $BG  ·  $N geometr$([[ $N -eq 1 ]] && echo y || echo ies)  ·  $N_REAL realizations each"
echo "  output: $OUT_DIR/<geom>_${BG}/"
echo

overall_t0=$SECONDS
done_cnt=0; skip_cnt=0
for (( gi=0; gi<N; gi++ )); do
  g="${GEOMS[$gi]}"
  gcsv="$OUT_DIR/${g}_${BG}/globals.csv"

  # resume: skip if already complete (expected row count present)
  if [[ -f "$gcsv" ]] && (( $(wc -l < "$gcsv") - 1 == expected )); then
    printf '[%d/%d] %s (%s)  already complete — skipping\n' "$(( gi + 1 ))" "$N" "$g" "$BG"
    skip_cnt=$(( skip_cnt + 1 ))
    continue
  fi

  printf '\n========== [%d/%d] geometry %s (%s) ==========\n' "$(( gi + 1 ))" "$N" "$g" "$BG"
  "$RUN_ONE" "$g" "$BG" "$N_REAL" "$SEED"
  done_cnt=$(( done_cnt + 1 ))
done

printf '\n  sweep done in %ds  (%d run, %d skipped, %d total geometries)\n' \
       "$(( SECONDS - overall_t0 ))" "$done_cnt" "$skip_cnt" "$N"
