#!/usr/bin/env bash
# Sriramajayam
#
# Embed-success sweep: concurrently choose the background resolution N and the
# admissible geometry set. For each N and each background type (tri, quad), run
# the robust admissibility gate (embed_check: K perturbed trials, reject on the
# first invalid embed) over every geometry, then report per-(N,background)
# pass/fail counts and the INTERSECTION admitted set (geometries that pass on
# BOTH backgrounds at that N — the common set the tri-vs-quad comparison needs).
#
# Decision rule (yours): pick the smallest N whose failure count is acceptably
# small; drop the few failing geometries. If many fail, go to the next-finer N.
#
# Output (under $OUT):
#   N<n>_<bg>.csv        per-geometry: stem,result,reason   (one file per N,bg)
#   admit_N<n>.txt       intersection admitted stems for resolution N
#   summary.csv          N,bg,pass,fail,total  (+ intersection rows bg=BOTH)
# Resumable: a geometry already recorded for an (N,bg) is skipped on re-run.

set -uo pipefail
shopt -s nullglob

# --------------------------------------------------------------------------- #
# Configuration (override via environment)
# --------------------------------------------------------------------------- #
N_LIST="${N_LIST:-64 128 256}"        # candidate resolutions (cells across [-1,1])
BG_TYPES="${BG_TYPES:-tri quad}"      # background element types
TRIALS="${TRIALS:-20}"                # perturbed trials per admissibility check
SEED="${SEED:-12345}"                 # base RNG seed for embed_check
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

# --------------------------------------------------------------------------- #
# Paths (resolved relative to this script; built binaries live in the build tree)
# --------------------------------------------------------------------------- #
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../../.." && pwd)"
# Built study binaries live in the build tree; scripts + (gitignored) data live
# here in the source tree. Override BIN if your build dir is not <repo>/build.
BIN="${BIN:-$REPO/build/performance/study/geometry}"
GEOM_DIR="${GEOM_DIR:-$HERE/data/geometries}"
BG_DIR="${BG_DIR:-$HERE/data/backgrounds}"
OUT="${OUT:-$HERE/data/sweep}"

EMBED_CHECK="$BIN/embed_check"
MAKE_BG="$BIN/make_background"

# --------------------------------------------------------------------------- #
if [[ -t 1 ]]; then
  BOLD=$'\e[1m'; DIM=$'\e[2m'; GREEN=$'\e[32m'; RED=$'\e[31m'; CYAN=$'\e[36m'; RESET=$'\e[0m'
else
  BOLD=''; DIM=''; GREEN=''; RED=''; CYAN=''; RESET=''
fi
die() { printf '%serror:%s %s\n' "$RED" "$RESET" "$*" >&2; exit 1; }

[[ -x "$EMBED_CHECK" ]] || die "embed_check not found at $EMBED_CHECK (build it first)"
[[ -x "$MAKE_BG"     ]] || die "make_background not found at $MAKE_BG (build it first)"
[[ -d "$GEOM_DIR"    ]] || die "geometry dir not found: $GEOM_DIR (run fset_to_polygon first)"

geoms=("$GEOM_DIR"/*.dat)
[[ ${#geoms[@]} -gt 0 ]] || die "no .dat geometries under $GEOM_DIR"

mkdir -p "$OUT" "$BG_DIR"

echo
echo "  ${BOLD}vemesh study · embed-success sweep${RESET}"
printf '  %-12s %s\n' "geometries" "${#geoms[@]}"
printf '  %-12s %s\n' "N"          "$N_LIST"
printf '  %-12s %s\n' "backgrounds" "$BG_TYPES"
printf '  %-12s %s (trials/gate)\n' "trials" "$TRIALS"
printf '  %-12s %s\n' "jobs"       "$JOBS"
printf '  %-12s %s\n' "output"     "$OUT"
echo

SUMMARY="$OUT/summary.csv"
echo "N,bg,pass,fail,total" > "$SUMMARY"

# one geometry's admissibility check -> writes "<stem>,<result>,<reason>" to a .res file
# args: <geom.dat> <bgfile> <resdir>
check_one() {
  local g="$1" bgfile="$2" resdir="$3"
  local stem; stem="$(basename "$g" .dat)"
  local res="$resdir/$stem.res"
  [[ -f "$res" ]] && return 0                 # resume: already done
  local log="$resdir/$stem.log"
  if OMP_NUM_THREADS=1 "$EMBED_CHECK" -g "$g" -i "$bgfile" -n "$TRIALS" -S "$SEED" >"$log" 2>&1; then
    printf '%s,pass,\n' "$stem" > "$res"
  else
    local reason; reason="$(grep -o 'reason=[^ ]*' "$log" | head -1)"
    printf '%s,fail,%s\n' "$stem" "${reason:-reason=unknown}" > "$res"
  fi
}
export -f check_one
export EMBED_CHECK TRIALS SEED

for N in $N_LIST; do
  for bg in $BG_TYPES; do
    bgfile="$BG_DIR/bg_${bg}_${N}.off"
    if [[ ! -f "$bgfile" ]]; then
      printf '  %sgenerating background %s...%s\n' "$DIM" "$(basename "$bgfile")" "$RESET"
      "$MAKE_BG" -n "$N" -t "$bg" -o "$BG_DIR" >/dev/null || die "make_background failed for N=$N $bg"
    fi
    resdir="$OUT/N${N}_${bg}"; mkdir -p "$resdir"
    printf '  %s[N=%s %s]%s checking %d geometries (%d jobs)...\n' \
           "$BOLD" "$N" "$bg" "$RESET" "${#geoms[@]}" "$JOBS"

    printf '%s\0' "${geoms[@]}" \
      | xargs -0 -P "$JOBS" -I{} bash -c 'check_one "$1" "$2" "$3"' _ {} "$bgfile" "$resdir"

    # aggregate this (N,bg)
    csv="$OUT/N${N}_${bg}.csv"; : > "$csv"
    cat "$resdir"/*.res >> "$csv"
    npass=$(grep -c ',pass,' "$csv" || true)
    nfail=$(grep -c ',fail,' "$csv" || true)
    printf '%s,%s,%s,%s,%s\n' "$N" "$bg" "$npass" "$nfail" "$((npass + nfail))" >> "$SUMMARY"
    printf '    %spass %s / fail %s%s\n' "$GREEN" "$npass" "$RED$nfail" "$RESET"
  done

  # intersection across the two backgrounds at this N (common admissible set)
  # (only meaningful when both tri and quad were run)
  tri_csv="$OUT/N${N}_tri.csv"; quad_csv="$OUT/N${N}_quad.csv"
  if [[ -f "$tri_csv" && -f "$quad_csv" ]]; then
    admit="$OUT/admit_N${N}.txt"
    comm -12 \
      <(awk -F, '$2=="pass"{print $1}' "$tri_csv" | sort) \
      <(awk -F, '$2=="pass"{print $1}' "$quad_csv" | sort) > "$admit"
    nboth=$(wc -l < "$admit" | tr -d ' ')
    printf '%s,%s,%s,%s,%s\n' "$N" "BOTH" "$nboth" "$(( ${#geoms[@]} - nboth ))" "${#geoms[@]}" >> "$SUMMARY"
    printf '  %s[N=%s]%s intersection admitted (tri AND quad): %s%d%s / %d  -> %s\n' \
           "$BOLD" "$N" "$RESET" "$CYAN" "$nboth" "$RESET" "${#geoms[@]}" "$admit"
  fi
  echo
done

echo "  ${BOLD}summary${RESET} ($SUMMARY):"
column -t -s, "$SUMMARY" | sed 's/^/    /'
echo
echo "  ${DIM}pick the smallest N whose failure count is acceptable; admit_N<n>.txt is the common set.${RESET}"
