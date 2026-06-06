#!/usr/bin/env bash
# Sriramajayam
#
# Performance test: sorgente meshes.
# For each sample_data/sorgente/*.off mesh, run vemesh_app three ways
#   - agglomeration only          (-a)
#   - relaxation only             (-r)
#   - agglomerate + relax (alt.)  (--ar)
# and stage baseline + improved meshes as VTK for the MATLAB VEM-conditioning
# evaluation in performance/matlab/.
#
# Output layout (mesh-first):
#   performance/output/sorgente/<name>/{baseline,agglomerate,relax,agglomerate_relax}.vtk
# In MATLAB, evaluate_directory(<name>) returns a 4-row table comparing the
# variants for that mesh.

set -euo pipefail
shopt -s nullglob

# --------------------------------------------------------------------------- #
# Configuration  (edit the meshes and algorithm options below)
# --------------------------------------------------------------------------- #

# meshes to process; empty => all *.off in $SORGENTE. Override with basenames:
#   MESHES=(mesh1_20 mesh2_20 mesh3_20)
MESHES=()

# Full vemesh_app option string per variant (everything except -i/-o, which the
# driver supplies). Include -m here; add -S <seed> on the relaxation variants for
# reproducibility (seed has no effect on agglomeration-only).
AGG_OPTS="-n 5 -q 0.2 -f 1.2 -m stability"                  # agglomeration only            (-a)
RELAX_OPTS="-n 5 -q 0.2 -s 20 -m stability -S 12345"        # relaxation only               (-r)
AR_OPTS="-n 5 -q 0.2 -f 1.2 -s 20 -m stability -S 12345"    # alternate agglomerate + relax (--ar)

# --------------------------------------------------------------------------- #
# Paths  (no edits needed; resolved relative to this script)
# --------------------------------------------------------------------------- #

# This script and its inputs/outputs all live under build/performance/ (copied
# there by CMake). Run it from there; everything resolves relative to $HERE,
# and outputs land under ./output (covered by the existing build/ gitignore).
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

VEMESH_APP="$HERE/vemesh_app"            # built alongside this script
SORGENTE="$HERE/sample_data/sorgente"    # copied in by CMake
OUT="$HERE/output/sorgente"

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

[[ -x "$VEMESH_APP" ]] || die "vemesh_app not found at $VEMESH_APP (build with -DBUILD_TESTS=ON)"
[[ -d "$SORGENTE"   ]] || die "sorgente dir not found at $SORGENTE"

if [[ ${#MESHES[@]} -eq 0 ]]; then
  for f in "$SORGENTE"/*.off; do MESHES+=("$(basename "$f" .off)"); done
fi
[[ ${#MESHES[@]} -gt 0 ]] || die "no .off meshes found in $SORGENTE"

mkdir -p "$OUT"

# banner
echo
echo "  ${BOLD}vemesh performance · sorgente${RESET}"
rule
printf '  %-10s %s\n' "meshes" "${#MESHES[@]}"
printf '  %-10s %s\n' "output" "$OUT"
echo
printf '  %-12s %s\n' "agglomerate"  "$DIM$AGG_OPTS$RESET"
printf '  %-12s %s\n' "relax"        "$DIM$RELAX_OPTS$RESET"
printf '  %-12s %s\n' "agg+relax"    "$DIM$AR_OPTS$RESET"
rule
echo

# run_variant <name> <variant> <mode-flag> <opts>
# runs vemesh_app quietly; prints one status line, dumps output only on failure
run_variant() {
  local name="$1" variant="$2" mode="$3" opts="$4"
  local run="$OUT/_runs/$name"          # one scratch dir per mesh, reused by all variants
  mkdir -p "$run"

  printf '    %-18s' "$variant"
  local t0=$SECONDS out
  if out=$("$VEMESH_APP" "$mode" -i "$SORGENTE/$name.off" -o "$run" $opts 2>&1); then
    cp "$run/output_mesh.vtk" "$OUT/$name/$variant.vtk"
    printf ' %s✓%s %s(%ds)%s\n' "$GREEN" "$RESET" "$DIM" "$((SECONDS - t0))" "$RESET"
  else
    printf ' %s✗%s\n' "$RED" "$RESET"
    printf '%s\n' "$out" | sed 's/^/      | /'
    die "$variant failed on $name"
  fi
}

i=0
for name in "${MESHES[@]}"; do
  i=$((i + 1))
  printf '  %s[%d/%d] %s%s\n' "$BOLD" "$i" "${#MESHES[@]}" "$name" "$RESET"
  rm -rf "$OUT/$name" "$OUT/_runs/$name"; mkdir -p "$OUT/$name"

  run_variant "$name" "agglomerate"        "-a"   "$AGG_OPTS"
  run_variant "$name" "relax"              "-r"   "$RELAX_OPTS"
  run_variant "$name" "agglomerate_relax"  "--ar" "$AR_OPTS"

  # baseline (the .off re-emitted as VTK) is identical across variants
  cp "$OUT/_runs/$name/input_mesh.vtk" "$OUT/$name/baseline.vtk"
  echo
done

# discard scratch; the curated <name>/ folders are all that's needed
rm -rf "$OUT/_runs"

rule
printf '  %s✓ done%s  %d mesh(es) staged under %s%s/<name>/%s\n' \
       "$GREEN$BOLD" "$RESET" "${#MESHES[@]}" "$CYAN" "$OUT" "$RESET"
echo "    baseline.vtk  agglomerate.vtk  relax.vtk  agglomerate_relax.vtk"
echo "  ${DIM}evaluate in MATLAB: performance/matlab/evaluate_directory${RESET}"
