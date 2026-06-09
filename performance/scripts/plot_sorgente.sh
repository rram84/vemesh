#!/usr/bin/env bash
# Sriramajayam
#
# Performance data + plots: sorgente — improvement strategies vs element count.
#
# Three figures, one per starting mesh:
#   full : improving the original mesh. Strategies = baseline (full, unimproved),
#          Sorgente_20 / Sorgente_40 (retain ~20%/40% of elements), and vemesh
#          agglomerate / relax / agglomerate_relax run on the full mesh.
#   _20  : vemesh improving the Sorgente 20%-retained mesh. Strategies = baseline
#          (the _20 mesh as-is) + vemesh agglomerate / relax / agglomerate_relax.
#   _40  : same as _20, for the 40%-retained mesh.
#
# Each figure groups data BY STRATEGY (one gnuplot 'index' block per strategy,
# one row per base mesh mesh1..mesh5), columns:
#   famidx  family  nelem  lambda_2  lambda_max  ratio
# and plots condition number (ratio = lambda_max/lambda_2, log-y) vs element
# count (log-x). A strategy keeps the same colour+marker across all three plots.
#
# Quality figures compare the per-element quality VECTORS (stability metric) of
# the strategies on one base mesh (default the most refined, QFAM=mesh5), one
# figure per starting mesh (full / Sorgente _20 / _40), read from the
# *_quality.dat files staged by run_sorgente.sh.
#
# Correlation figure tests whether the element-level optimization shows up in the
# assembled-level conditioning. The optimization acts per element; the only way
# that can move the spectrum is through lambda_max, set by the worst element.
# The bare-stiffness condition number lambda_max/lambda_2 also carries an h^-2
# resolution growth (lambda_2 ~ h^2 ~ 1/N_vert, fitted exponent -0.95, r=-0.9998
# against vertex count), which is unrelated to element quality. Dividing it out
# gives a dimensionless, resolution-independent measure that pools cleanly across
# the whole suite (R^2 ~ 0.98). So we scatter:
#   y = (lambda_max/lambda_2) / N_vert   (scale-corrected condition number)
#   x = worst element quality = min of the *_quality.dat stability column
# N_vert is the vertex count (POINTS in the VTK); it is invariant across the
# variants of a mesh, so the normalisation does not differ between baseline /
# agglomerate / relax of the same mesh. Points are coloured by the same six
# strategies as the other figures (baseline/Sorgente_20/Sorgente_40 = the
# unimproved full/20%/40% meshes; agglomerate/relax/agglomerate_relax pooled over
# starting meshes), with a log-log best-fit line overlaid.
# across ALL sorgente cases (every family x starting-mesh x variant), one point
# per case, coloured by the per-element operation (baseline / agglomerate / relax
# / agglomerate_relax). A clean downward trend means worst-element quality
# governs lambda_max, i.e. element-level work does control assembled conditioning.
#
# Inputs : output/sorgente/eigen.csv (analyze_sorgente.sh), the staged VTKs
#          (element counts) and the *_quality.dat files.
# Output : output/sorgente/sorgente_cases_{full,20,40}.{dat,<ext>}
#          output/sorgente/quality_<QFAM>_{full,20,40}.<ext>
#          output/sorgente/correlation_scaledcond_vs_minq.{dat,<ext>}

set -euo pipefail
shopt -s nullglob

# --------------------------------------------------------------------------- #
# Config
# --------------------------------------------------------------------------- #
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUT="$HERE/output/sorgente"
CSV="$OUT/eigen.csv"

# paper-quality vector PDF by default; raster preview via TERM_GP / GP_SIZE.
TERM_GP="${TERM_GP:-pdfcairo}"
GP_SIZE="${GP_SIZE:-8in,5.5in}"
case "$TERM_GP" in pdfcairo) EXT=pdf ;; pngcairo) EXT=png ;; *) EXT=out ;; esac

# starting meshes to plot (conditioning figures)
INPUTS=(full _20 _40)

# base mesh whose element-quality vectors are compared (most refined by default),
# and which starting meshes to compare quality for (full / Sorgente _20 / _40).
QFAM="${QFAM:-mesh5}"
QINPUTS=(full _20 _40)

# --------------------------------------------------------------------------- #
command -v gnuplot >/dev/null 2>&1 || { echo "gnuplot not found in PATH" >&2; exit 1; }
[[ -f "$CSV" ]] || { echo "no eigen.csv at $CSV (run analyze_sorgente.sh first)" >&2; exit 1; }

families=$(awk -F, 'NR>1{m=$1; sub(/_[0-9]+$/,"",m); print m}' "$CSV" | sort -u | xargs)
nfam=$(echo $families | wc -w | tr -d ' ')

# fetch "lambda_2 lambda_max ratio" for a (mesh,variant) from eigen.csv
eig() { awk -F, -v m="$1" -v V="$2" 'NR>1 && $1==m && $2==V {print $3, $4, $5; exit}' "$CSV"; }
# element (POLYGON) count of a staged VTK
nel() { awk '/^POLYGONS/{print $2; exit}' "$1"; }
# vertex (POINTS) count of a staged VTK
npts() { awk '/^POINTS/{print $2; exit}' "$1"; }

# per-strategy style: "rgb-fill filled-pt open-pt"  (consistent across all plots)
style_for() {
  case "$1" in
    baseline)          echo "#7fc7c0 7 6"  ;;   # teal,       circle
    Sorgente_20)       echo "#f7a072 5 4"  ;;   # peach,      square
    Sorgente_40)       echo "#8fa6dd 9 8"  ;;   # periwinkle, triangle
    agglomerate)       echo "#e79bc9 11 10" ;;  # pink,       inv-triangle
    relax)             echo "#b3d56a 13 12" ;;  # green,      diamond
    agglomerate_relax) echo "#e9c46a 15 14" ;;  # gold,       pentagon
  esac
}

# --------------------------------------------------------------------------- #
# Build + render one figure for a given starting mesh.
# --------------------------------------------------------------------------- #
make_figure() {
  local input="$1"
  local tag="${input#_}"                 # full / 20 / 40
  local dat="$OUT/sorgente_cases_${tag}.dat"
  local fig="$OUT/sorgente_cases_${tag}.${EXT}"
  local title suffix
  local -a STRATEGIES

  if [[ "$input" == full ]]; then
    title="sorgente: improving the full base mesh"
    suffix=""
    STRATEGIES=(
      "baseline||baseline"
      "Sorgente_20|_20|baseline"
      "Sorgente_40|_40|baseline"
      "agglomerate||agglomerate"
      "relax||relax"
      "agglomerate_relax||agglomerate_relax"
    )
  else
    title="sorgente: vemesh improving the Sorgente ${tag}% meshes"
    suffix="$input"                       # _20 or _40
    STRATEGIES=(
      "baseline|${suffix}|baseline"
      "agglomerate|${suffix}|agglomerate"
      "relax|${suffix}|relax"
      "agglomerate_relax|${suffix}|agglomerate_relax"
    )
  fi
  local nstrat=${#STRATEGIES[@]}

  # data: one index block per strategy, one row per base mesh
  : > "$dat"
  printf '# famidx  family  nelem  lambda_2  lambda_max  ratio\n' >> "$dat"
  for s in "${STRATEGIES[@]}"; do
    local stitle smesh_suffix svar
    IFS='|' read -r stitle smesh_suffix svar <<< "$s"
    printf '# strategy: %s\n' "$stitle" >> "$dat"
    local fi=0
    for fam in $families; do
      fi=$((fi + 1))
      local mesh="${fam}${smesh_suffix}"
      local row; row=$(eig "$mesh" "$svar")
      [[ -n "$row" ]] || { echo "warn: no eigen row for $mesh/$svar" >&2; continue; }
      local vtk="$OUT/$mesh/$svar.vtk"
      [[ -f "$vtk" ]] || { echo "warn: missing $vtk" >&2; continue; }
      printf '%d %s %s %s\n' "$fi" "$fam" "$(nel "$vtk")" "$row" >> "$dat"
    done
    printf '\n\n' >> "$dat"
  done

  # style lists in strategy order
  local titles="" colors="" fmark="" omark=""
  for s in "${STRATEGIES[@]}"; do
    local t="${s%%|*}" c fm om
    read -r c fm om <<< "$(style_for "$t")"
    titles+="$t "; colors+="$c "; fmark+="$fm "; omark+="$om "
  done

  echo "wrote $dat"
  gnuplot <<GP
set terminal ${TERM_GP} enhanced size ${GP_SIZE} font "Helvetica,11" linewidth 1.2
set output "${fig}"

set logscale xy
set format x "10^{%T}"
set format y "10^{%T}"
set ylabel "condition number ({/Symbol l}_{max}/{/Symbol l}_{min})"
set xlabel "# elements"
set border lc rgb "#666666"
set grid xtics ytics lc rgb "#e3e3e3"
set key outside right top noenhanced
set title "${title}"

titles  = "${titles}"
colors  = "${colors}"
fmark   = "${fmark}"
omark   = "${omark}"
psz     = 1.5

# pass 1: pastel filled markers + connecting line (legend); pass 2: dark outline.
plot for [i=0:${nstrat}-1] "${dat}" index i using 3:6 \
       with linespoints lw 2 pt int(word(fmark,i+1)) ps psz \
       lc rgb word(colors,i+1) title word(titles, i+1), \
     for [i=0:${nstrat}-1] "${dat}" index i using 3:6 \
       with points pt int(word(omark,i+1)) ps psz lw 1.1 lc rgb "#555555" notitle
GP
  echo "wrote $fig"
}

# --------------------------------------------------------------------------- #
# Element-quality vectors for one base mesh: quality (log-y) vs sorted element
# index (log-x), one line per strategy. Reads the *_quality.dat files staged by
# run_sorgente.sh (column 1 = rank, column 2 = stability quality); no VTKs.
# --------------------------------------------------------------------------- #
make_quality_figure() {
  local fam="$1" input="$2"
  local tag="${input#_}"                       # full / 20 / 40
  local fig="$OUT/quality_${fam}_${tag}.${EXT}"
  local ftitle
  local -a QCASES
  if [[ "$input" == full ]]; then
    ftitle="sorgente ${fam}: quality vectors -- improving the full mesh"
    QCASES=(
      "baseline|${fam}/baseline_quality.dat"
      "Sorgente_20|${fam}_20/baseline_quality.dat"
      "Sorgente_40|${fam}_40/baseline_quality.dat"
      "agglomerate|${fam}/agglomerate_quality.dat"
      "relax|${fam}/relax_quality.dat"
      "agglomerate_relax|${fam}/agglomerate_relax_quality.dat"
    )
  else
    ftitle="sorgente ${fam}: quality vectors -- improving the Sorgente ${tag}% mesh"
    QCASES=(
      "baseline|${fam}${input}/baseline_quality.dat"
      "agglomerate|${fam}${input}/agglomerate_quality.dat"
      "relax|${fam}${input}/relax_quality.dat"
      "agglomerate_relax|${fam}${input}/agglomerate_relax_quality.dat"
    )
  fi

  local plotcmd="" c title rel f col
  for c in "${QCASES[@]}"; do
    IFS='|' read -r title rel <<< "$c"
    f="$OUT/$rel"
    [[ -f "$f" ]] || { echo "warn: missing $f (skipping $title)" >&2; continue; }
    col=$(style_for "$title"); col="${col%% *}"     # first field of style = fill colour
    plotcmd+="'$f' using 1:2 with lines lw 2.5 lc rgb '$col' title '$title', "
  done
  [[ -n "$plotcmd" ]] || { echo "warn: no quality-vector files for $fam $input (skipping)" >&2; return; }
  plotcmd="${plotcmd%, }"

  gnuplot <<GP
set terminal ${TERM_GP} enhanced size ${GP_SIZE} font "Helvetica,11" linewidth 1.2
set output "${fig}"
set logscale xy
set format x "10^{%T}"
set format y "10^{%T}"
set xlabel "element index (sorted by quality)"
set ylabel "element quality (stability)"
set border lc rgb "#666666"
set grid xtics ytics lc rgb "#e3e3e3"
set key bottom right noenhanced
set title "${ftitle}"
plot ${plotcmd}
GP
  echo "wrote $fig"
}

# --------------------------------------------------------------------------- #
# Correlation: scale-corrected condition number (lambda_max/lambda_2)/N_vert
# (dimensionless, resolution-independent, log-y) vs worst element quality
# (log-x), one point per sorgente case (family x starting-mesh x variant).
#
# The cloud maps onto the SAME six strategies as the other figures, so each gets
# its own style_for colour AND marker (no separate level channel, no grey):
#   - the unimproved meshes (baseline operation) on the full / 20% / 40% starting
#     meshes ARE the baseline / Sorgente_20 / Sorgente_40 strategies;
#   - agglomerate / relax / agglomerate_relax are one strategy each, pooled over
#     all three starting meshes.
# A log-log least-squares line through all points is overlaid (slope + R^2 shown).
#
# ratio from eigen.csv; N_vert = POINTS in the VTK; worst quality = min of the
# stability column in the *_quality.dat. One index block per strategy, in the
# style order below.
# --------------------------------------------------------------------------- #
make_correlation_figure() {
  local dat="$OUT/correlation_scaledcond_vs_minq.dat"
  local fig="$OUT/correlation_scaledcond_vs_minq.${EXT}"

  # strategy = "title|inputs|operation"; inputs is a space-separated list of the
  # starting meshes pooled into that strategy (matches style_for keys for colour).
  local -a STRATEGIES=(
    "baseline|full|baseline"
    "Sorgente_20|_20|baseline"
    "Sorgente_40|_40|baseline"
    "agglomerate|full _20 _40|agglomerate"
    "relax|full _20 _40|relax"
    "agglomerate_relax|full _20 _40|agglomerate_relax"
  )
  local nstrat=${#STRATEGIES[@]}

  : > "$dat"
  printf '# worst_quality  scaled_cond=(lmax/lmin)/Nvert  family  input  variant\n' >> "$dat"
  for s in "${STRATEGIES[@]}"; do
    local stitle sinputs sop
    IFS='|' read -r stitle sinputs sop <<< "$s"
    printf '# strategy: %s\n' "$stitle" >> "$dat"
    local input
    for input in $sinputs; do
      local suffix=""; [[ "$input" == full ]] || suffix="$input"
      for fam in $families; do
        local mesh="${fam}${suffix}"
        local row; row=$(eig "$mesh" "$sop")
        [[ -n "$row" ]] || { echo "warn: no eigen row for $mesh/$sop" >&2; continue; }
        local cond; cond=$(echo "$row" | awk '{print $3}')   # lambda_2 lambda_max ratio
        local vtk="$OUT/$mesh/$sop.vtk"
        [[ -f "$vtk" ]] || { echo "warn: missing $vtk" >&2; continue; }
        local nv; nv=$(npts "$vtk")          # vertex count for the h^-2 scaling
        [[ -n "$nv" && "$nv" -gt 0 ]] || { echo "warn: no vertex count in $vtk" >&2; continue; }
        local scond; scond=$(awk -v c="$cond" -v v="$nv" 'BEGIN{print c/v}')
        local qf="$OUT/$mesh/${sop}_quality.dat"
        [[ -f "$qf" ]] || { echo "warn: missing $qf" >&2; continue; }
        # worst element quality = min of the stability column (col 2)
        local wq; wq=$(awk '$1 ~ /^[0-9]/ {q=$2; if(m==""||q<m)m=q} END{print m}' "$qf")
        [[ -n "$wq" ]] || { echo "warn: empty quality in $qf" >&2; continue; }
        printf '%s %s %s %s %s\n' "$wq" "$scond" "$fam" "$input" "$sop" >> "$dat"
      done
    done
    printf '\n\n' >> "$dat"
  done

  # style lists in strategy order (colour + filled marker + open outline marker)
  local titles="" colors="" fmark="" omark=""
  for s in "${STRATEGIES[@]}"; do
    local t="${s%%|*}" c fm om
    read -r c fm om <<< "$(style_for "$t")"
    titles+="$t "; colors+="$c "; fmark+="$fm "; omark+="$om "
  done

  # log-log least-squares fit through ALL plotted points -> slope A, intercept B
  # (in log10), and R^2, passed to gnuplot to draw y = 10^B * x^A.
  local A B R2
  read -r A B R2 < <(awk 'NF>=5 && $1 ~ /^[0-9eE.+-]+$/ {
      lx=log($1)/log(10); ly=log($2)/log(10);
      n++; sx+=lx; sy+=ly; sxx+=lx*lx; sxy+=lx*ly; syy+=ly*ly
    } END {
      s=(n*sxy-sx*sy)/(n*sxx-sx*sx); b=(sy-s*sx)/n;
      r=(n*sxy-sx*sy)/sqrt((n*sxx-sx*sx)*(n*syy-sy*sy));
      printf "%.6f %.6f %.4f\n", s, b, r*r
    }' "$dat")

  echo "wrote $dat   (fit: slope=$A, R^2=$R2)"
  gnuplot <<GP
set terminal ${TERM_GP} enhanced size ${GP_SIZE} font "Helvetica,11" linewidth 1.2
set output "${fig}"
set logscale xy
set format x "10^{%T}"
set format y "10^{%T}"
set xlabel "worst element quality (min stability)"
set ylabel "scale-corrected condition number  ({/Symbol l}_{max}/{/Symbol l}_{min}) / N_{vert}"
set border lc rgb "#666666"
set grid xtics ytics lc rgb "#e3e3e3"
set key outside right top noenhanced
set title "sorgente: scale-corrected condition number vs worst element quality"

titles = "${titles}"
colors = "${colors}"
fmark  = "${fmark}"
omark  = "${omark}"
psz    = 1.4

A = ${A}
B = ${B}
fitline(x) = (10**B) * x**A      # NB: 'fit' is a reserved gnuplot keyword

# best-fit power law (dashed), then per-strategy pastel filled markers + dark
# open-outline overlay. Each strategy keeps its colour+marker from the others.
plot fitline(x) with lines dt 2 lw 2 lc rgb "#444444" \
       title sprintf("fit: slope %.2f, R^2 %.2f", A, ${R2}), \
     for [i=0:${nstrat}-1] "${dat}" index i using 1:2 \
       with points pt int(word(fmark,i+1)) ps psz lc rgb word(colors,i+1) title word(titles,i+1), \
     for [i=0:${nstrat}-1] "${dat}" index i using 1:2 \
       with points pt int(word(omark,i+1)) ps psz lw 1.1 lc rgb "#555555" notitle
GP
  echo "wrote $fig"
}

for input in "${INPUTS[@]}"; do
  make_figure "$input"
done

for input in "${QINPUTS[@]}"; do
  make_quality_figure "$QFAM" "$input"
done

make_correlation_figure
