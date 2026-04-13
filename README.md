# He3BeamSim — Geant4 Beam Broadening Simulation

## Physics problem

A 6 MeV ³He beam (diameter **0.8 cm**, uniform disk) passes through:
1. A **1 µm Si₃N₄** (silicon nitride) window
2. **1 m of vacuum**

The simulation records the transverse beam profile at the scoring plane
(z = 1 m after the window) and reports σ_x, σ_y, FWHM, and containment radii.

The dominant broadening mechanism is **Coulomb multiple scattering** in the
Si₃N₄ foil.  The Highland formula gives a rough analytical estimate:

    θ₀ = (13.6 MeV / βcp) · z · √(x/X₀) · [1 + 0.038 ln(x/X₀)]

For 6 MeV ³He (Z=2, A=3):
  - βcp ≈ 110 MeV (relativistic momentum × velocity for a 6 MeV He-3)
  - x/X₀ for 1 µm Si₃N₄ ≈ 9.35e-6 / 65.5e-3 ≈ 1.43e-4  (density 3.17 g/cm³, X₀ ≈ 65 mm)
  - θ₀ ≈ (13.6 × 2 / 110) × √(1.43e-4) ≈ 0.00132 rad ≈ 1.32 mrad

After 1 m drift:  Δr ≈ θ₀ × 1 m ≈ 1.3 mm (1σ broadening added in quadrature)

The Geant4 simulation computes this rigorously, including the full
angular distribution (not just the Gaussian core).

---

## File structure

```
He3BeamSim/
├── CMakeLists.txt
├── He3BeamSim.cc               ← main driver
├── run.mac                     ← batch macro (100 000 events)
├── vis.mac                     ← interactive visualisation macro
├── plot_beam_profile.py        ← Python post-processing & plots
├── include/
│   ├── DetectorConstruction.hh
│   ├── PrimaryGeneratorAction.hh
│   ├── ActionInitialization.hh
│   ├── RunAction.hh
│   ├── EventAction.hh
│   └── SteppingAction.hh
└── src/
    ├── DetectorConstruction.cc
    ├── PrimaryGeneratorAction.cc
    ├── ActionInitialization.cc
    ├── RunAction.cc
    ├── EventAction.cc
    └── SteppingAction.cc
```

---

## Requirements

- **Geant4 ≥ 10.7** (tested with 10.7, 11.x)
- CMake ≥ 3.16
- C++17 compiler (gcc ≥ 9, clang ≥ 10)
- ROOT (optional but recommended — for `.root` histogram output)
- Python 3 + numpy + matplotlib + scipy (for `plot_beam_profile.py`)
  - If ROOT output: also install `uproot`

---

## Build

```bash
# 1. Source your Geant4 environment
source /path/to/geant4/install/bin/geant4.sh
# or on some systems:
# source /path/to/geant4/install/share/Geant4-*/geant4make/geant4make.sh

# 2. Configure
cd He3BeamSim
mkdir build && cd build
cmake ..

# 3. Compile
make -j$(nproc)
```

---

## Run

### Batch mode (recommended, 100 000 events ≈ 1–5 min)

```bash
cd build
./He3BeamSim ../run.mac
```

Output printed to console:
```
======================================================
  He-3  6 MeV  |  1 µm Si3N4  +  1 m vacuum
  Beam profile at scoring plane (z = 1 m)
  Events processed: 100000
------------------------------------------------------
  Mean X     :   -0.0012 mm
  Sigma X    :    4.1523 mm
  FWHM  X    :    9.7784 mm
  Mean Y     :    0.0008 mm
  Sigma Y    :    4.1490 mm
  FWHM  Y    :    9.7707 mm
  Mean R     :    5.2814 mm
  RMS  R     :    1.9801 mm
------------------------------------------------------
  Initial beam radius (hard edge): 4.000 mm
======================================================
```
*(Numbers above are illustrative; your exact results depend on Geant4 version and random seed.)*

### Interactive / visualisation mode

```bash
./He3BeamSim          # opens Qt or X11 viewer
```

---

## Post-processing

```bash
# With ROOT output
python3 ../plot_beam_profile.py He3Beam.root

# With CSV output (if Geant4 built without ROOT)
python3 ../plot_beam_profile.py He3Beam_hits.csv
```

Produces three PNG files:
- `beam_profile_xy.png`   — 2D hexbin transverse profile
- `beam_profile_r.png`    — radial distribution + 90% containment circle
- `beam_profile_proj.png` — x and y projections with Gaussian fits

---

## Physics notes

### Geometry

| Component      | Material | Thickness | Position        |
|----------------|----------|-----------|-----------------|
| World volume   | Vacuum   | 120 cm    | tube, R=5 cm    |
| Si₃N₄ window  | Si₃N₄    | 1 µm      | z = 0           |
| Drift region   | Vacuum   | 1 m       | z = 0 → 1 m     |
| Scoring plane  | Si (thin)| 1 µm      | z = 1 m         |

### Physics list: QBBC

`QBBC` is Geant4's recommended list for ion beams. It includes:

| Component                    | Relevance here                              |
|------------------------------|---------------------------------------------|
| `G4EmStandardPhysics_option4`| Urban MSC model — best accuracy for ions    |
| `G4IonPhysics`               | He-3 nuclear interactions                   |
| `G4IonElasticPhysics`        | Elastic He-3 scattering                     |
| `G4HadronInelasticQBBC`      | Hadronic inelastic (rare at 6 MeV)          |

The Urban multiple-scattering model correctly handles the
**non-Gaussian tails** (large-angle Rutherford scatters) that
contribute to the halo beyond the core Gaussian.

### Step limiter

`/run/setCut 0.01 mm` ensures the MSC angular deflection is sampled
in sub-steps through the 1 µm window.  Without this, a single step
spanning the entire foil would under-sample the scattering.

### Production cut

The default 0.01 mm production cut suppresses delta-ray production in
the thin foil, keeping the simulation fast while preserving accuracy
for the primary He-3 track.

---

## Tuning suggestions

| Goal | Change |
|------|--------|
| Higher accuracy | Use `G4EmStandardPhysics_option4` explicitly, add `G4UrbanMscModel` parameters |
| Faster run | Reduce events to 10 000 (statistics still good for σ) |
| Thicker window | Change `windowHz` in `DetectorConstruction.cc` |
| Different energy | Change `SetParticleEnergy` in `PrimaryGeneratorAction.cc` |
| Proton beam | Change ion definition to `Z=1, A=1` |
| Different drift | Change `driftLen` in `DetectorConstruction.cc` |
| Divergent beam | Add angular spread in `PrimaryGeneratorAction.cc` |
