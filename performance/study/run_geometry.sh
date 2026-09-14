#!/usr/bin/env bash
# Sriramajayam
#
# Relaxation-vs-agglomeration study (Computational Mechanics paper) -- ONE
# geometry, end to end.
#
# For a fixed (geometry, background) pair:
#   1. embed_shapes generates N perturbed embeddings (the statistical sampler of
#      curve-cell intersections; perturbs background nodes near the interface).
#   2. each embedding is improved by vemesh_app under BOTH driving metrics
#      (stability, shape) and ALL FOUR workflows (relax, agglomerate, ra, ar),
#      capturing the mesh after EACH operation (-v op -> 6 captures/workflow).
#   3. every mesh is analyzed the instant it is produced -- ONE mesh_analyze pass
#      (C++) gives the global VEM conditioning plus counts and BOTH element
#      metrics on the ALTERED elements -- then the mesh is DELETED. Nothing kept.
#
# Parallelism: realizations are independent, so they run as a CONTINUOUS POOL of
# JOBS single-threaded workers (xargs -P; each worker sets OMP_NUM_THREADS=1, so
# concurrent workers fill the cores without oversubscribing vemesh's own OpenMP).
# A continuous pool (rather than fixed batches) keeps every core busy to the end
# -- no barrier stalls, no ragged final batch. Each worker re-invokes this script
# in a hidden "__worker" mode for one realization, writing its own part files.
#
# Everything scalar is appended to two CSVs (the only things kept):
#   output/<geom>_<bg>/globals.csv  geom,bg,driver,workflow,real,op,step,nelems,nverts,
#                                   n_alt_faces,n_alt_verts,lambda2,lambda_max,cond_ratio
#   output/<geom>_<bg>/altered.csv  key,step,n_alt, extremes(qs/qg min,max),
#                                   sums(qs,qg,qs^2,qg^2,qs*qg), and FIXED-bin
#                                   histograms of q_stability, q_geom and sides,
#                                   pooled over realizations per (geom,bg,driver,
#                                   workflow,step). Geometries are NOT merged here.
#
# Usage:  ./run_geometry.sh [GEOM] [BG] [N_REAL] [SEED]
#   GEOM    geometry stem under dataset/geometries (default 85909)
#   BG      background: quad | tri                 (default quad)
#   N_REAL  number of perturbed realizations       (default 20)
#   SEED    base RNG seed                          (default 12345)
#   JOBS    (env) size of the worker pool          (default = CPU count)

set -euo pipefail
shopt -s nullglob

# ---- mode dispatch: "__worker <geom> <bg> <n_real> <seed> <i>" runs ONE realization ---
MODE="main"
if [[ "${1:-}" == "__worker" ]]; then MODE="worker"; shift; fi

# --------------------------------------------------------------------------- #
# Arguments / configuration (side-effect free; shared by main and workers)
# --------------------------------------------------------------------------- #
GEOM="${1:-85909}"
BG="${2:-quad}"
N_REAL="${3:-20}"
SEED="${4:-12345}"
WI="${5:-}"                    # worker realization index (worker mode only)

JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 8)}"
(( JOBS < 1 )) && JOBS=1

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

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
EMBED="$REPO/build/performance/mesh_generators/embed_shapes"
APP="$REPO/build/performance/vemesh_app"
ANALYZE="$REPO/build/performance/study/mesh_analyze"
DATASET="$HERE/dataset"
GEOM_FILE="$DATASET/geometries/$GEOM.dat"
BG_FILE="$DATASET/backgrounds/bg_${BG}_128.off"

OUT="$HERE/output/${GEOM}_${BG}"
PARTS="$OUT/_parts"           # per-realization part files
WORK="$OUT/_work"             # per-realization scratch (meshes; transient)
G_CSV="$OUT/globals.csv"
A_CSV="$OUT/altered.csv"

die() { printf 'error: %s\n' "$*" >&2; exit 1; }
[[ -x "$EMBED"     ]] || die "embed_shapes not built at $EMBED"
[[ -x "$APP"       ]] || die "vemesh_app not built at $APP"
[[ -x "$ANALYZE"   ]] || die "mesh_analyze not built at $ANALYZE"
[[ -f "$GEOM_FILE" ]] || die "geometry not found: $GEOM_FILE"
[[ -f "$BG_FILE"   ]] || die "background not found: $BG_FILE"

# --------------------------------------------------------------------------- #
# step: linear operation index in EXECUTION order, so "quantity vs step" is a
# clean x-axis per workflow. -r/-a: step=iter k. --ra: r then a -> 2k,2k+1.
# --ar: a then r -> 2k,2k+1. base: -1.
# --------------------------------------------------------------------------- #
step_of() {
  local wf="$1" op="$2" k t
  if [[ "$op" == base ]]; then echo -1; return; fi
  k="${op#iter}"; k="${k%-*}"; t="${op##*-}"
  case "$wf" in
    relax|agglomerate) echo "$k" ;;
    ra) [[ "$t" == r ]] && echo $(( 2*k )) || echo $(( 2*k + 1 )) ;;
    ar) [[ "$t" == a ]] && echo $(( 2*k )) || echo $(( 2*k + 1 )) ;;
  esac
}

# --------------------------------------------------------------------------- #
# Analyze ONE mesh immediately in a SINGLE process (mesh_analyze reads the mesh
# once): conditioning + counts -> G globals row; altered-set histograms -> H raw
# row. G_CSV / RAW_A here are the worker's OWN part files.
# --------------------------------------------------------------------------- #
analyze_mesh() {
  local f="$1" driver="$2" wf="$3" real="$4" op="$5" step
  step="$(step_of "$wf" "$op")"

  "$ANALYZE" -i "$f" --tag t --altered-stats \
    | awk -F, -v g="$G_CSV" -v raw="$RAW_A" -v k="$GEOM,$BG,$driver,$wf,$real,$op" -v step="$step" \
          'BEGIN{OFS=","}
        $1=="G"{ print k,step,$3,$4,$5,$6,$7,$8,$9 >> g }
        $1=="H"{ out=k","step; for(i=3;i<=NF;i++) out=out","$i; print out >> raw }' \
    || die "mesh_analyze failed on $f"
}

# --------------------------------------------------------------------------- #
# Run ONE realization end to end, into this worker's own part files. Every mesh
# is deleted after analysis; nothing is retained.
# --------------------------------------------------------------------------- #
run_realization() {
  local i="$1"
  export OMP_NUM_THREADS=1
  local seed_i=$(( SEED + i ))
  local wd="$WORK/w$i"; rm -rf "$wd"; mkdir -p "$wd/embed" "$wd/run"
  local EMB="$wd/embed" RUN="$wd/run"
  G_CSV="$PARTS/globals_$i.csv"; RAW_A="$PARTS/rawH_$i.csv"; : > "$G_CSV"; : > "$RAW_A"

  "$EMBED" -g "$GEOM_FILE" -i "$BG_FILE" -o "$EMB" -n 1 -S "$seed_i" >/dev/null 2>&1 \
    || die "embed_shapes failed for $GEOM realization $i"
  local emb="$EMB/embed-0.vtk"
  [[ -f "$emb" ]] || die "expected embedded mesh missing: $emb"

  analyze_mesh "$emb" baseline none "$i" base

  local m entry name flag iters cap op
  for m in "${DRIVERS[@]}"; do
    for entry in "${WORKFLOWS[@]}"; do
      IFS='|' read -r name flag iters <<< "$entry"
      rm -rf "$RUN"; mkdir -p "$RUN"
      case "$name" in
        relax)       "$APP" "$flag" -i "$emb" -o "$RUN" -n "$iters" -q "$QEPS"              -s "$NSAMP" -m "$m" -S "$seed_i" -v op >/dev/null 2>&1 ;;
        agglomerate) "$APP" "$flag" -i "$emb" -o "$RUN" -n "$iters" -q "$QEPS" -f "$QFAC"               -m "$m"             -v op >/dev/null 2>&1 ;;
        *)           "$APP" "$flag" -i "$emb" -o "$RUN" -n "$iters" -q "$QEPS" -f "$QFAC" -s "$NSAMP" -m "$m" -S "$seed_i" -v op >/dev/null 2>&1 ;;
      esac || die "vemesh_app $flag ($name,$m) failed on $GEOM realization $i"
      for cap in "$RUN"/mesh-iter*.vtk; do
        op="$(basename "$cap" .vtk)"; op="${op#mesh-}"
        analyze_mesh "$cap" "$m" "$name" "$i" "$op"
      done
      rm -rf "$RUN"
    done
  done
  rm -rf "$wd"
  printf '    [%s/%s] realization %d/%d done\n' "$GEOM" "$BG" "$(( i + 1 ))" "$N_REAL"
}

# ---- worker mode: one realization, then exit (part dirs already exist) ------
if [[ "$MODE" == worker ]]; then
  [[ -n "$WI" ]] || die "worker mode requires a realization index"
  run_realization "$WI"
  exit 0
fi

# =========================== main =========================================== #
rm -rf "$OUT"; mkdir -p "$OUT" "$PARTS" "$WORK"
echo "geom,bg,driver,workflow,real,op,step,nelems,nverts,n_alt_faces,n_alt_verts,lambda2,lambda_max,cond_ratio" > "$G_CSV"

echo
echo "  relax-vs-agglomerate study  ·  geometry $GEOM  ·  background $BG"
echo "  realizations=$N_REAL  seed=$SEED  drivers=${DRIVERS[*]}  jobs=$JOBS (continuous pool)"
echo "  output: $OUT"
echo

overall_t0=$SECONDS

# continuous pool of JOBS workers over realizations 0..N_REAL-1 (no barriers).
seq 0 $(( N_REAL - 1 )) \
  | xargs -P "$JOBS" -I{} "$0" __worker "$GEOM" "$BG" "$N_REAL" "$SEED" {} \
  || die "one or more realization workers failed"

# concatenate per-worker part files, then pool the H rows into altered.csv.
cat "$PARTS"/globals_*.csv >> "$G_CSV" 2>/dev/null || true
RAW_A="$OUT/_altered_raw.csv"
cat "$PARTS"/rawH_*.csv > "$RAW_A" 2>/dev/null || : > "$RAW_A"

# KEEP_RAW=1 also writes altered_raw.csv: the UNPOOLED per-realization H rows
# (keyed by ...,real,op,step). Use it on a pilot geometry to test histogram
# convergence vs #realizations; omit it for the full study (pooled only = small).
RAW_OUT=""
[[ "${KEEP_RAW:-0}" == 1 ]] && RAW_OUT="$OUT/altered_raw.csv"

# --------------------------------------------------------------------------- #
# Pool per-mesh H rows into altered.csv: one row per (geom,bg,driver,workflow,
# step), summed over realizations (histograms + sums add; extremes take min/max).
# Geometries are NOT merged here. QNB/SNB must match mesh_analyze.cpp. With a
# non-empty 3rd arg, also emit the per-realization rows verbatim (with header).
# --------------------------------------------------------------------------- #
python3 - "$RAW_A" "$A_CSV" "$RAW_OUT" <<'PY'
import sys
QNB, SNB = 1000, 48          # must match mesh_analyze.cpp
raw, out = sys.argv[1], sys.argv[2]
raw_out = sys.argv[3] if len(sys.argv) > 3 else ""

# fixed histogram-bin column names, shared by the pooled and per-realization files
bincols  = ["qs_h%d" % i for i in range(QNB)]
bincols += ["qg_h%d" % i for i in range(QNB)]
bincols += ["s%d" % (3+i) for i in range(SNB-1)] + ["s%dp" % (3+SNB-1)]
stat_hdr = ["n_alt","qs_min","qs_max","qg_min","qg_max","qs_sum","qg_sum","qs_sumsq","qg_sumsq","qsqg_sum"]

rawfh = open(raw_out, "w") if raw_out else None
if rawfh:
    rawfh.write(",".join(["geom","bg","driver","workflow","real","op","step"] + stat_hdr + bincols) + "\n")

groups, order = {}, []
try:
    fh = open(raw)
except FileNotFoundError:
    fh = None
if fh:
    for line in fh:
        t = line.rstrip("\n").split(",")
        if len(t) < 17:
            continue
        if rawfh:
            rawfh.write(",".join(t) + "\n")           # per-realization row, verbatim
        key = (t[0], t[1], t[2], t[3], t[6])          # geom,bg,driver,workflow,step
        n_alt = int(float(t[7]))
        ext = [float(t[8]), float(t[9]), float(t[10]), float(t[11])]   # qs_min,qs_max,qg_min,qg_max
        sums = [float(x) for x in t[12:17]]           # qs_sum,qg_sum,qs_sumsq,qg_sumsq,qsqg_sum
        hist = [int(float(x)) for x in t[17:]]        # QNB + QNB + SNB
        if key not in groups:
            order.append(key)
            groups[key] = [n_alt, ext[:], sums[:], hist[:]]
        else:
            g = groups[key]
            g[0] += n_alt
            g[1][0] = min(g[1][0], ext[0]); g[1][1] = max(g[1][1], ext[1])
            g[1][2] = min(g[1][2], ext[2]); g[1][3] = max(g[1][3], ext[3])
            for i in range(len(sums)): g[2][i] += sums[i]
            for i in range(len(hist)): g[3][i] += hist[i]
    fh.close()
if rawfh:
    rawfh.close()
hdr = ["geom","bg","driver","workflow","step"] + stat_hdr + bincols
fmt = lambda x: ("%.10g" % x) if isinstance(x, float) else str(x)
with open(out, "w") as o:
    o.write(",".join(hdr) + "\n")
    for key in order:
        n_alt, ext, sums, hist = groups[key]
        row = list(key) + [n_alt] + ext + sums + hist
        o.write(",".join(fmt(v) for v in row) + "\n")
PY
rm -rf "$PARTS" "$WORK" "$RAW_A"

printf '\n  done in %ds  (%d realizations, %d-way continuous pool)\n' \
       "$(( SECONDS - overall_t0 ))" "$N_REAL" "$JOBS"
printf '  globals=%d rows (per mesh)   altered=%d rows (pooled per driver x workflow x step)\n' \
       "$(( $(wc -l < "$G_CSV") - 1 ))" "$(( $(wc -l < "$A_CSV") - 1 ))"
echo "  -> $OUT/{globals,altered}.csv"
