#!/usr/bin/env python3
# Sriramajayam
#
# Performance plots: clipped circle. Two figures from the staged data:
#
# 1) DISTRIBUTION (circle_condition_dist): grouped VIOLIN plots with the subdivision
#    levels along the x-axis and, within each level, four violins -- one per variant
#    -- showing the spread of conditioning over the random clippings. A dark dot
#    marks the median, a thick bar the IQR. The y-axis is the condition number
#    SCALED BY h^2: bare-stiffness conditioning grows as h^-2 with refinement
#    (lambda_min ~ h^2), so multiplying by h^2 removes that drift and puts the levels
#    on one footing. The KDE is computed in log10 space and the axis is linear in
#    log10 with powers-of-ten tick labels, so the density is not warped by a log axis.
#    h(level) = H0 * 2^-level (level 0 = H0; each level halves h), per run_circle.sh.
#
# 2) CORRELATION (circle_correlation_minq): does element-level optimization show up in
#    assembled-level conditioning? Scatter pooled over every level x realization x
#    variant of
#       y = (lambda_max / lambda_2) / N_vert      scale-corrected condition number
#       x = worst element quality = min per-element stability ratio
#    with a reference -1/2 power law overlaid. N_vert removes the h^-2 resolution
#    growth, so the cloud is dimensionless and pools across levels.
#
# Data sources (no MATLAB, no re-run):
#   - ratio (lambda_max/lambda_2) from analyze_circle.sh's per-level CSV;
#   - N_vert from the VTK POINTS count;
#   - worst element quality from the VTK's embedded `face_quality` CELL_DATA -- the
#     improved variants are written AFTER vemesh_app's quality evaluation (runs used
#     `-m stability`, so face_quality IS the per-element stability ratio); baseline
#     likewise carries it because run_circle.sh stages vemesh_app's input_mesh.vtk
#     (which embeds face_quality). Any mesh lacking it is silently skipped (counted).
#
# Inputs : output/circle/<level>.csv (analyze_circle.sh) + the staged VTKs
# Output : output/circle/circle_condition_dist.<ext>
#          output/circle/circle_correlation_minq.<ext>     (PDF by default)

import csv
import math
import os
import sys

import numpy as np
import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Patch
from matplotlib.ticker import FixedLocator, FuncFormatter

# --------------------------------------------------------------------------- #
# Config
# --------------------------------------------------------------------------- #
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "output", "circle")

EXT = os.environ.get("PLOT_EXT", "pdf")           # pdf (paper) or png (preview)
DIST_FIG = os.path.join(OUT, f"circle_condition_dist.{EXT}")
CORR_FIG = os.path.join(OUT, f"circle_correlation_minq.{EXT}")

# variant order and shared colours/markers (consistent across all figures).
VARIANTS = ["baseline", "agglomerate", "relax", "agglomerate_relax"]
COLOURS = {
    "baseline":          "#7fc7c0",   # teal
    "agglomerate":       "#e79bc9",   # pink
    "relax":             "#b3d56a",   # green
    "agglomerate_relax": "#e9c46a",   # gold
}
MARKERS = {
    "baseline":          "o",         # circle
    "agglomerate":       "v",         # inverted triangle
    "relax":             "D",         # diamond
    "agglomerate_relax": "p",         # pentagon
}

# background mesh size at level 0; h(level) = H0 * 2^-level (each level halves h).
# Must match run_circle.sh; override with BG_H0=... if you changed it there.
H0 = float(os.environ.get("BG_H0", "0.2"))

DX = 0.17          # spacing between adjacent violins within a level group
WIDTH = 0.15       # max violin width (must stay < DX so neighbours don't merge)


def die(msg):
    sys.stderr.write(f"error: {msg}\n")
    sys.exit(1)


def level_key(stem):
    """Sort levels numerically when they are integers, else lexicographically."""
    try:
        return (0, int(stem))
    except ValueError:
        return (1, stem)


def h_for(level):
    """Mesh size for a level, or None if the level stem is not an integer."""
    try:
        return H0 * 2.0 ** (-int(level))
    except ValueError:
        return None


def parse_vtk(path):
    """Return (min_face_quality, n_points) from a legacy VTK, or (None, None) if the
    face_quality CELL_DATA block is absent. Reads only as far as that block."""
    npts = ncells = None
    minq = None
    with open(path) as fh:
        it = iter(fh)
        for line in it:
            s = line.split()
            if not s:
                continue
            if s[0] == "POINTS":
                npts = int(s[1])
            elif s[0] == "CELL_DATA":
                ncells = int(s[1])
            elif s[0] == "SCALARS" and len(s) >= 2 and s[1] == "face_quality":
                next(it, None)                       # skip LOOKUP_TABLE line
                need = ncells or 0
                got = 0
                m = None
                while got < need:
                    toks = next(it).split()
                    for tok in toks:
                        x = float(tok)
                        if m is None or x < m:
                            m = x
                        got += 1
                        if got >= need:
                            break
                minq = m
                break
    return minq, npts


# --------------------------------------------------------------------------- #
# Discover levels from the per-level CSVs (shared by both figures)
# --------------------------------------------------------------------------- #
if not os.path.isdir(OUT):
    die(f"no output/circle at {OUT} (run analyze_circle.sh first)")

levels = sorted(
    (os.path.splitext(os.path.basename(p))[0]
     for p in (os.path.join(OUT, f) for f in os.listdir(OUT))
     if p.endswith(".csv")),
    key=level_key,
)
if not levels:
    die(f"no per-level CSVs under {OUT} (run analyze_circle.sh first)")


def pow10_yaxis(ax, lo, hi):
    """Powers-of-ten tick labels for a linear-in-log10 y-axis."""
    kmin, kmax = int(math.floor(lo)), int(math.ceil(hi))
    ax.yaxis.set_major_locator(FixedLocator(list(range(kmin, kmax + 1))))
    ax.yaxis.set_major_formatter(FuncFormatter(lambda v, _p: rf"$10^{{{int(round(v))}}}$"))


# =========================================================================== #
# Figure 1: distribution (violins of h^2-scaled condition number per level)
# =========================================================================== #
def make_distribution_figure():
    # level -> variant -> log10(cond * h^2) array
    data = {}
    warned_h = False
    for level in levels:
        vals = {v: [] for v in VARIANTS}
        with open(os.path.join(OUT, f"{level}.csv"), newline="") as fh:
            reader = csv.reader(fh)
            next(reader, None)
            for row in reader:
                if len(row) < 5:
                    continue
                variant = row[1]
                try:
                    ratio = float(row[4])
                except ValueError:
                    continue
                if variant in vals and ratio > 0 and math.isfinite(ratio):
                    vals[variant].append(math.log10(ratio))
        # h^2 scaling is an additive offset in log10: log10(cond*h^2) = log10(cond) + 2 log10(h)
        h = h_for(level)
        if h is None:
            if not warned_h:
                sys.stderr.write(f"warn: non-integer level {level!r}; leaving it unscaled\n")
                warned_h = True
            off = 0.0
        else:
            off = 2.0 * math.log10(h)
        data[level] = {v: np.asarray(a) + off for v, a in vals.items() if len(a) >= 2}

    if not any(data[l] for l in levels):
        sys.stderr.write(f"warn: no usable rows for the distribution figure; skipping\n")
        return

    def draw_violins(target, dx, width):
        half = (len(VARIANTS) - 1) * dx / 2.0
        for li, level in enumerate(levels, start=1):
            for vi, variant in enumerate(VARIANTS):
                arr = data[level].get(variant)
                if arr is None or arr.size < 2:
                    continue
                pos = li - half + vi * dx
                parts = target.violinplot(
                    arr, positions=[pos], widths=width,
                    showmeans=False, showmedians=False, showextrema=False,
                )
                for body in parts["bodies"]:
                    body.set_facecolor(COLOURS[variant])
                    body.set_edgecolor("#555555")
                    body.set_alpha(0.85)
                    body.set_linewidth(0.6)
                q1, med, q3 = np.percentile(arr, [25, 50, 75])
                target.vlines(pos, q1, q3, color="#333333", lw=4, zorder=3)
                target.plot(pos, med, "o", mfc="white", mec="#222222", ms=4, mew=1.0, zorder=4)

    fig, ax = plt.subplots(figsize=(11.0, 5.5))
    draw_violins(ax, DX, WIDTH)

    all_vals = np.concatenate([a for l in levels for a in data[l].values()])
    lo, hi = all_vals.min(), all_vals.max()
    pad = 0.05 * (hi - lo if hi > lo else 1.0)
    ax.set_ylim(lo - pad, hi + pad)
    pow10_yaxis(ax, lo, hi)

    ax.set_xlim(0.5, len(levels) + 0.5)
    ax.set_xticks(range(1, len(levels) + 1))
    ax.set_xticklabels(levels)
    ax.set_xlabel("subdivision level")
    ax.set_ylabel(r"scaled condition number  ($h^{2}\,\lambda_{\max}/\lambda_{\min}$)")
    ax.set_title("clipped circle: distribution of conditioning over random realizations")

    ax.grid(axis="y", color="#e3e3e3", lw=0.8)
    ax.set_axisbelow(True)
    for sp in ("top", "right"):
        ax.spines[sp].set_visible(False)
    for sp in ("left", "bottom"):
        ax.spines[sp].set_color("#666666")

    handles = [Patch(facecolor=COLOURS[v], edgecolor="#555555", alpha=0.85, label=v) for v in VARIANTS]
    ax.legend(handles=handles, loc="upper left", bbox_to_anchor=(1.01, 1.0),
              frameon=False, title="variant")

    fig.tight_layout()
    fig.savefig(DIST_FIG, bbox_inches="tight")
    print(f"wrote {DIST_FIG}")


# =========================================================================== #
# Figure 2: correlation (worst element quality vs scale-corrected conditioning)
# =========================================================================== #
def make_correlation_figure():
    pts = {v: {"x": [], "y": []} for v in VARIANTS}    # worst quality, scaled cond
    nmiss = 0
    for level in levels:
        with open(os.path.join(OUT, f"{level}.csv"), newline="") as fh:
            reader = csv.reader(fh)
            next(reader, None)
            for row in reader:
                if len(row) < 5:
                    continue
                realn, variant = row[0], row[1]
                if variant not in VARIANTS:
                    continue
                try:
                    ratio = float(row[4])
                except ValueError:
                    continue
                if not (ratio > 0 and math.isfinite(ratio)):
                    continue
                vtk = os.path.join(OUT, level, f"{realn}__{variant}.vtk")
                if not os.path.isfile(vtk):
                    nmiss += 1
                    continue
                minq, npts = parse_vtk(vtk)
                if minq is None or not npts or minq <= 0:
                    nmiss += 1
                    continue
                pts[variant]["x"].append(minq)
                pts[variant]["y"].append(ratio / npts)

    total = sum(len(pts[v]["x"]) for v in VARIANTS)
    if total == 0:
        sys.stderr.write("warn: no (quality, conditioning) pairs; skipping correlation figure "
                         "(re-run run_circle.sh / check face_quality)\n")
        return
    if nmiss:
        sys.stderr.write(f"note: skipped {nmiss} rows (missing VTK or no face_quality)\n")

    # The fit characterises the NATURAL quality -> conditioning relationship, so it
    # is computed over the BASELINE cases ONLY: the improving operations deliberately
    # bunch points into the favourable (high-quality, low-cond) corner, which would
    # bias a pooled fit. The fitted lines are drawn across the full x-range so the
    # improved variants can be read against the baseline trend. Two lines: a FIXED
    # -1/2 reference (offset only) and the FREE least-squares best fit.
    allx = np.concatenate([np.asarray(pts[v]["x"]) for v in VARIANTS if pts[v]["x"]])

    fig, ax = plt.subplots(figsize=(8.5, 6.0))
    for v in VARIANTS:
        if not pts[v]["x"]:
            continue
        ax.scatter(pts[v]["x"], pts[v]["y"], s=16, marker=MARKERS[v],
                   facecolor=COLOURS[v], edgecolor="none", alpha=0.35, label=v)

    bx = np.asarray(pts["baseline"]["x"])
    by = np.asarray(pts["baseline"]["y"])
    A_fit = B_fit = R2_fit = R2_ref = float("nan")
    if bx.size >= 2:
        lx, ly = np.log10(bx), np.log10(by)
        sst = float(np.sum((ly - ly.mean()) ** 2))

        def r2(A, B):
            return 1.0 - float(np.sum((ly - (A * lx + B)) ** 2)) / sst if sst > 0 else float("nan")

        A_ref = -0.5
        B_ref = float(np.mean(ly - A_ref * lx))                  # fixed slope, offset only
        R2_ref = r2(A_ref, B_ref)
        A_fit, B_fit = (float(c) for c in np.polyfit(lx, ly, 1))  # free best fit
        R2_fit = r2(A_fit, B_fit)

        xs = np.logspace(math.log10(allx.min()), math.log10(allx.max()), 100)
        ax.plot(xs, (10 ** B_ref) * xs ** A_ref, ls="--", lw=2, color="#999999",
                label=fr"slope $-1/2$ (baseline $R^2$ {R2_ref:.2f})")
        ax.plot(xs, (10 ** B_fit) * xs ** A_fit, ls="-", lw=2, color="#444444",
                label=fr"baseline best fit: slope {A_fit:.2f} ($R^2$ {R2_fit:.2f})")
    else:
        sys.stderr.write("note: <2 baseline points; drawing the cloud without a fit line "
                         "(re-run run_circle.sh so baseline carries face_quality)\n")

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("worst element quality (min stability)")
    ax.set_ylabel(r"scale-corrected condition number  $(\lambda_{\max}/\lambda_{\min})\,/\,N_{\mathrm{vert}}$")
    ax.set_title("clipped circle: scale-corrected condition number vs worst element quality")

    ax.grid(True, which="major", color="#e3e3e3", lw=0.8)
    ax.set_axisbelow(True)
    for sp in ("top", "right"):
        ax.spines[sp].set_visible(False)
    for sp in ("left", "bottom"):
        ax.spines[sp].set_color("#666666")

    leg = ax.legend(loc="best", frameon=False)
    for h in leg.legend_handles:
        if hasattr(h, "set_alpha"):
            h.set_alpha(0.9)

    fig.tight_layout()
    fig.savefig(CORR_FIG, bbox_inches="tight")
    print(f"wrote {CORR_FIG}   (baseline best-fit slope {A_fit:.3f}, R^2 {R2_fit:.3f}; "
          f"-1/2 ref R^2 {R2_ref:.3f}; {total} points, {bx.size} baseline)")


# --------------------------------------------------------------------------- #
make_distribution_figure()
make_correlation_figure()
print("levels:", " ".join(levels))
