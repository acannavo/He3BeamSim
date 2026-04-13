#!/usr/bin/env python3
"""
plot_beam_profile.py
====================
Post-process the ROOT (or CSV ntuple) output from He3BeamSim.

If ROOT output is available:
    python3 plot_beam_profile.py He3Beam.root

If you built Geant4 with CSV output (no ROOT):
    python3 plot_beam_profile.py He3Beam_hits.csv

Produces:
  - beam_profile_xy.png  : 2D scatter / hexbin
  - beam_profile_r.png   : radial distribution with Gaussian fit
  - beam_profile_proj.png: x and y projections with Gaussian fits
"""

import sys
import numpy as np
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
from scipy.stats import norm

# ── helpers ──────────────────────────────────────────────────────────────────

def gaussian(x, A, mu, sigma):
    return A * np.exp(-0.5 * ((x - mu) / sigma) ** 2)

def load_csv(path):
    """Load CSV ntuple written by G4AnalysisManager (skip header lines)."""
    data = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.startswith('x'):
                continue
            parts = line.split(',')
            if len(parts) >= 3:
                try:
                    data.append([float(p) for p in parts[:3]])
                except ValueError:
                    continue
    arr = np.array(data)
    return arr[:, 0], arr[:, 1], arr[:, 2]   # x, y, r  (mm)

def load_root(path):
    import uproot
    with uproot.open(path) as f:
        tree = f["hits"]
        x = tree["x"].array(library="np")
        y = tree["y"].array(library="np")
        r = tree["r"].array(library="np")
    return x, y, r

# ── main ─────────────────────────────────────────────────────────────────────

def main():
    path = sys.argv[1] if len(sys.argv) > 1 else "He3Beam_hits.csv"

    if path.endswith(".root"):
        x, y, r = load_root(path)
    else:
        x, y, r = load_csv(path)

    N = len(x)
    print(f"Loaded {N} hits from {path}")

    sx = np.std(x);  mx = np.mean(x)
    sy = np.std(y);  my = np.mean(y)
    sr = np.std(r);  mr = np.mean(r)
    fwhm_x = 2.3548 * sx
    fwhm_y = 2.3548 * sy

    print(f"\nBeam statistics at scoring plane (z = 1 m after Si3N4 window)")
    print(f"  N hits     = {N}")
    print(f"  Mean X     = {mx:.4f} mm     Sigma X = {sx:.4f} mm   FWHM X = {fwhm_x:.4f} mm")
    print(f"  Mean Y     = {my:.4f} mm     Sigma Y = {sy:.4f} mm   FWHM Y = {fwhm_y:.4f} mm")
    print(f"  Mean R     = {mr:.4f} mm     RMS  R  = {sr:.4f} mm")

    # 90% containment radius
    r_sorted = np.sort(r)
    r90 = r_sorted[int(0.90 * N)]
    print(f"  90% containment R = {r90:.4f} mm")
    print(f"\n  Initial beam radius (hard edge) = 4.000 mm")
    print(f"  Added broadening in x (approx)  = {np.sqrt(max(0, sx**2 - (4.0/np.sqrt(2))**2)):.4f} mm")

    # ── Plot 1: 2D profile ──────────────────────────────────────────────────
    fig, ax = plt.subplots(figsize=(6, 5))
    h = ax.hexbin(x, y, gridsize=80, cmap='inferno', mincnt=1)
    cb = fig.colorbar(h, ax=ax, label='Counts')
    ax.set_xlabel('x  [mm]')
    ax.set_ylabel('y  [mm]')
    ax.set_title('He-3 beam  |  Transverse profile at z = 1 m\n'
                 f'(6 MeV, Ø 8 mm initial beam, 1 µm Si₃N₄ window)')
    ax.set_aspect('equal')
    # Draw initial beam edge
    circle = plt.Circle((0, 0), 4.0, color='cyan', fill=False,
                         linewidth=1.5, linestyle='--', label='Initial beam edge')
    ax.add_patch(circle)
    ax.legend(fontsize=9)
    fig.tight_layout()
    fig.savefig('beam_profile_xy.png', dpi=150)
    print("Saved beam_profile_xy.png")
    plt.close(fig)

    # ── Plot 2: radial distribution ─────────────────────────────────────────
    fig, ax = plt.subplots(figsize=(6, 4))
    bins = np.linspace(0, 10, 100)
    counts, edges = np.histogram(r, bins=bins)
    centers = 0.5 * (edges[:-1] + edges[1:])
    ax.bar(centers, counts, width=(bins[1]-bins[0]), alpha=0.6,
           color='steelblue', label='Simulation')
    ax.axvline(4.0, color='red', linestyle='--', linewidth=1.5,
               label='Initial beam radius (4 mm)')
    ax.axvline(r90, color='orange', linestyle=':', linewidth=1.5,
               label=f'90% containment ({r90:.2f} mm)')
    ax.set_xlabel('Radial position r  [mm]')
    ax.set_ylabel('Counts')
    ax.set_title('Radial beam distribution at scorer')
    ax.legend(fontsize=9)
    fig.tight_layout()
    fig.savefig('beam_profile_r.png', dpi=150)
    print("Saved beam_profile_r.png")
    plt.close(fig)

    # ── Plot 3: x and y projections ─────────────────────────────────────────
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4))
    brange = (-10, 10)
    nbins  = 100
    for ax, vals, label, mean, sigma in [
            (ax1, x, 'x', mx, sx),
            (ax2, y, 'y', my, sy)]:
        cnts, edg = np.histogram(vals, bins=nbins, range=brange)
        ctrs = 0.5 * (edg[:-1] + edg[1:])
        ax.bar(ctrs, cnts, width=(edg[1]-edg[0]), alpha=0.6,
               color='steelblue', label='Simulation')
        # Gaussian fit
        try:
            popt, _ = curve_fit(gaussian, ctrs, cnts,
                                p0=[cnts.max(), mean, sigma])
            xfit = np.linspace(*brange, 400)
            ax.plot(xfit, gaussian(xfit, *popt), 'r-', linewidth=1.8,
                    label=f'Gauss fit\nµ={popt[1]:.3f}, σ={popt[2]:.3f} mm\nFWHM={2.3548*popt[2]:.3f} mm')
        except Exception:
            pass
        ax.set_xlabel(f'{label}  [mm]')
        ax.set_ylabel('Counts')
        ax.set_title(f'{label} projection')
        ax.legend(fontsize=8)

    fig.suptitle('He-3 beam projections at z = 1 m (6 MeV, 1 µm Si₃N₄ + 1 m vacuum)',
                 fontsize=11)
    fig.tight_layout()
    fig.savefig('beam_profile_proj.png', dpi=150)
    print("Saved beam_profile_proj.png")
    plt.close(fig)

if __name__ == "__main__":
    main()
