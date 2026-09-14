#!/usr/bin/env bash
# Sriramajayam
#
# Relaxation-vs-agglomeration study (Computational Mechanics paper) -- ONE
# geometry, end to end. Stage 1 driver: get every detail right on a single
# geometry before generalizing to the full set.
#
# For a fixed (geometry, background) pair:
#   1. embed_shapes generates N perturbed embeddings (the statistical sampler of
#      curve-cell intersections; perturbs background nodes near the interface).
#   2. each embedding is improved by vemesh_app under BOTH driving metrics
#      (stability, shape) and ALL FOUR workflows (relax, agglomerate, ra, ar),
#      capturing the mesh after EACH operation (-v op -> 6 captures/workflow).
#   3. every mesh is analyzed the instant it is produced -- mesh_conditioning
#      (C++) for the global VEM conditioning, mesh_metrics for counts + BOTH
#      element metrics on the ALTERED elements -- then DELETED. Because the C++
#      conditioner is a fast per-mesh call (no MATLAB engine to amortize), there
#      is no staging or batching: a mesh lives only long enough to be measured.
#
# Everything scalar is appended to two CSVs (the only things kept):
#   output/<geom>_<bg>/globals.csv    geom,bg,driver,workflow,real,op,nelems,nverts,
#                                     n_alt_faces,n_alt_verts,lambda2,lambda_max,cond_ratio
#   output/<geom>_<bg>/perturbed.csv  geom,bg,driver,workflow,real,op,face_idx,sides,q_stability,q_geom
# op="base" is the unimproved embedded baseline. The two share the key columns
# (geom,bg,driver,workflow,real,op); perturbed joins to globals on that key.
#
# Usage:  ./run_geometry.sh [GEOM] [BG] [N_REAL] [SEED]
#   GEOM    geometry stem under dataset/geometries (default 85909)
#   BG      background: quad | tri                 (default quad)
#   N_REAL  number of perturbed realizations       (default 20)
#   SEED    base RNG seed                          (default 12345)

set -euo pipefail
shopt -s nullglob

# --------------------------------------------------------------------------- #
# Arguments / configuration
# --------------------------------------------------------------------------- #
GEOM="${1:-85909}"
BG="${2:-quad}"
N_REAL="${3:-20}"
SEED="${4:-12345}"

# shared vemesh_app options (per the locked design)
QEPS="0.2"; QFAC="1.2"; NSAMP="20"

# workflows as "name|flag|iters" (bash 3.2: indexed array, no assoc arrays).
# All four do equal work = 6 atomic operations, so -v op yields 6 captures each.
WORKFLOWS=(
  "relax|-r|6"
  "agglomerate|-a|6"
  "ra|--ra|3"
  "ar|--ar|3"
)
DRIVERS=(stability shape)

# --------------------------------------------------------------------------- #
# Paths (resolved relative to this script; repo layout is fixed)
# --------------------------------------------------------------------------- #
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"

EMBED="$REPO/build/performance/mesh_generators/embed_shapes"
APP="$REPO/build/performance/vemesh_app"
METRICS="$REPO/build/performance/study/mesh_metrics"
CONDITION="$REPO/build/performance/study/mesh_conditioning"

DATASET="$HERE/dataset"
GEOM_FILE="$DATASET/geometries/$GEOM.dat"
BG_FILE="$DATASET/backgrounds/bg_${BG}_128.off"

OUT="$HERE/output/${GEOM}_${BG}"

# --------------------------------------------------------------------------- #
# Sanity checks
# --------------------------------------------------------------------------- #
die() { printf 'error: %s\n' "$*" >&2; exit 1; }
[[ -x "$EMBED"     ]] || die "embed_shapes not built at $EMBED"
[[ -x "$APP"       ]] || die "vemesh_app not built at $APP"
[[ -x "$METRICS"   ]] || die "mesh_metrics not built at $METRICS"
[[ -x "$CONDITION" ]] || die "mesh_conditioning not built at $CONDITION"
[[ -f "$GEOM_FILE" ]] || die "geometry not found: $GEOM_FILE"
[[ -f "$BG_FILE"   ]] || die "background not found: $BG_FILE"

# --------------------------------------------------------------------------- #
# Fresh output; CSV headers written once
# --------------------------------------------------------------------------- #
rm -rf "$OUT"; mkdir -p "$OUT"
RUN="$OUT/_run"              # vemesh_app scratch (per invocation)
EMB="$OUT/_embed"            # the current realization's embedded mesh

G_CSV="$OUT/globals.csv"
echo "geom,bg,driver,workflow,real,op,nelems,nverts,n_alt_faces,n_alt_verts,lambda2,lambda_max,cond_ratio" > "$G_CSV"
P_CSV="$OUT/perturbed.csv"
echo "geom,bg,driver,workflow,real,op,face_idx,sides,q_stability,q_geom" > "$P_CSV"

echo
echo "  relax-vs-agglomerate study  ·  geometry $GEOM  ·  background $BG"
echo "  realizations=$N_REAL  seed=$SEED  drivers=${DRIVERS[*]}"
echo "  output: $OUT"
echo

t_run=0; t_analyze=0
overall_t0=$SECONDS

# --------------------------------------------------------------------------- #
# Analyze ONE mesh immediately (conditioning + counts + per-altered quality) and
# append its rows, keyed by (geom,bg,driver,workflow,real,op). Called the instant
# a mesh is produced; the caller deletes the file afterwards.
#   globals.csv <- one row: counts + lambda2,lambda_max,cond_ratio
#   perturbed.csv <- one row per ALTERED face: sides,q_stability,q_geom
# --------------------------------------------------------------------------- #
analyze_mesh() {
  local f="$1" driver="$2" wf="$3" real="$4" op="$5" t0
  t0=$SECONDS

  # global VEM conditioning: "<name>.vtk,lambda2,lambda_max,ratio"
  local cl l2 lmax ratio
  cl="$("$CONDITION" "$f")" || die "mesh_conditioning failed on $f"
  l2="${cl#*,}"; lmax="${l2#*,}"; ratio="${lmax#*,}"; lmax="${lmax%%,*}"; l2="${l2%%,*}"

  # counts + per-altered-element quality (both metrics); merge cond into the G row
  "$METRICS" -i "$f" --tag t --emit-faces \
    | awk -F, -v g="$G_CSV" -v p="$P_CSV" -v k="$GEOM,$BG,$driver,$wf,$real,$op" \
          -v l2="$l2" -v lmax="$lmax" -v ratio="$ratio" 'BEGIN{OFS=","}
        $1=="G"{ print k,$3,$4,$5,$6,l2,lmax,ratio >> g }
        $1=="F"{ print k,$3,$4,$5,$6 >> p }' \
    || die "mesh_metrics failed on $f"

  t_analyze=$(( t_analyze + SECONDS - t0 ))
}

# --------------------------------------------------------------------------- #
# Driver: one realization at a time. Embed -> analyze baseline -> for each
# (driver, workflow) run vemesh_app and analyze each capture as it appears.
# Nothing is retained: the embedding and every capture are deleted after use.
# --------------------------------------------------------------------------- #
for (( i=0; i<N_REAL; i++ )); do
  seed_i=$(( SEED + i ))                       # per-realization seed (embed + relax)

  rm -rf "$EMB"; mkdir -p "$EMB"
  "$EMBED" -g "$GEOM_FILE" -i "$BG_FILE" -o "$EMB" -n 1 -S "$seed_i" >/dev/null 2>&1 \
    || die "embed_shapes failed for $GEOM realization $i"
  emb="$EMB/embed-0.vtk"
  [[ -f "$emb" ]] || die "expected embedded mesh missing: $emb"

  # baseline (driver-independent, no optimization)
  analyze_mesh "$emb" baseline none "$i" base

  for m in "${DRIVERS[@]}"; do
    for entry in "${WORKFLOWS[@]}"; do
      IFS='|' read -r name flag iters <<< "$entry"
      rm -rf "$RUN"; mkdir -p "$RUN"
      # per-workflow flags: -f only when agglomeration is involved; -s/-S only when relaxation is.
      local_t0=$SECONDS
      case "$name" in
        relax)       "$APP" "$flag" -i "$emb" -o "$RUN" -n "$iters" -q "$QEPS"              -s "$NSAMP" -m "$m" -S "$seed_i" -v op >/dev/null 2>&1 ;;
        agglomerate) "$APP" "$flag" -i "$emb" -o "$RUN" -n "$iters" -q "$QEPS" -f "$QFAC"               -m "$m"             -v op >/dev/null 2>&1 ;;
        *)           "$APP" "$flag" -i "$emb" -o "$RUN" -n "$iters" -q "$QEPS" -f "$QFAC" -s "$NSAMP" -m "$m" -S "$seed_i" -v op >/dev/null 2>&1 ;;
      esac || die "vemesh_app $flag ($name,$m) failed on $GEOM realization $i"
      t_run=$(( t_run + SECONDS - local_t0 ))

      # analyze each per-operation capture (mesh-iterK-{a,r}.vtk) immediately
      for cap in "$RUN"/mesh-iter*.vtk; do
        op="$(basename "$cap" .vtk)"; op="${op#mesh-}"     # -> iterK-a / iterK-r
        analyze_mesh "$cap" "$m" "$name" "$i" "$op"
      done
      rm -rf "$RUN"
    done
  done
  rm -rf "$EMB"

  printf '  realization %d/%d done\n' "$(( i + 1 ))" "$N_REAL"
done

printf '\n  done in %ds  (vemesh_app=%ds  analyze=%ds)\n' \
       "$(( SECONDS - overall_t0 ))" "$t_run" "$t_analyze"
printf '  globals=%d  perturbed=%d rows\n' \
       "$(( $(wc -l < "$G_CSV") - 1 ))" "$(( $(wc -l < "$P_CSV") - 1 ))"
echo "  -> $OUT/{globals,perturbed}.csv"
