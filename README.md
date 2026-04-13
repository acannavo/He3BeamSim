# He3BeamSim — Geant4 Beam Broadening Simulation

## Physics problem

A 6 MeV ³He beam passes through:
1. A **Si₃N₄** (silicon nitride) entrance window
2. A **He-4 gas volume** at configurable pressure
3. A **vacuum/gas drift** to a scoring plane

All geometry parameters — beam size, foil dimensions, gas pressure, drift distance, and scorer size — are controlled from a single plain-text configuration file (`sim.conf`) with no recompilation required.

The dominant broadening mechanism is **Coulomb multiple scattering** in the Si₃N₄ foil, with additional (typically small) scattering in the He-4 gas depending on pressure and drift length.

---

## Configuration file

All geometry and physics parameters are set in `sim.conf`, located in the project root:

```
# He3BeamSim configuration file
#
BeamD   =   20.0    # beam diameter          (mm)
FoilD   =   30.0    # Si3N4 foil diameter    (mm)  — must be ≥ BeamD
FoilT   =    1.0    # Si3N4 foil thickness   (um)
PipeD   =   60.0    # He4 gas pipe diameter  (mm)
Dist    =  500.0    # foil-to-scorer distance (mm)
ScorerD =  200.0    # scorer diameter        (mm)
GasPres =    5.0    # He4 gas pressure       (Torr)
```

**Key rules:**
- `FoilD` must be ≥ `BeamD` — if the beam is wider than the foil, particles outside the foil skip scattering and a hollow ring appears at the scorer.
- `PipeD` sets the radius of the He-4 gas cylinder between foil and scorer. Particles outside this radius travel through vacuum.
- `ScorerD` is independent of `PipeD` — the scorer can be wider than the gas tube to catch scattered particles.
- `GasPres` sets the He-4 pressure in Torr. The gas density is computed automatically from the ideal gas law at 293 K.

Edit `sim.conf` and re-run — no recompilation needed.

---

## File structure

```
He3BeamSim/
├── CMakeLists.txt
├── He3BeamSim.cc               ← main driver
├── sim.conf                    ← geometry & physics parameters
├── run.mac                     ← batch macro (100 000 events)
├── vis.mac                     ← interactive visualisation macro
├── init_vis.mac                ← default geometry for interactive mode
├── plot_beam_profile.py        ← Python post-processing & plots
├── include/
│   ├── SimConfig.hh            ← config file reader (singleton)
│   ├── DetectorConstruction.hh
│   ├── PrimaryGeneratorAction.hh
│   ├── ActionInitialization.hh
│   ├── RunAction.hh
│   ├── EventAction.hh
│   └── SteppingAction.hh
└── src/
    ├── SimConfig.cc
    ├── DetectorConstruction.cc
    ├── PrimaryGeneratorAction.cc
    ├── ActionInitialization.cc
    ├── RunAction.cc
    ├── EventAction.cc
    └── SteppingAction.cc
```

---

## Requirements

- **Geant4 11.2** (tested with 11.2.2)
- CMake ≥ 3.16
- C++17 compiler (gcc ≥ 9)
- ROOT (required — for `.root` histogram and PNG output)
- Python 3 + numpy + matplotlib + scipy (for `plot_beam_profile.py`)

---

## Build

```bash
source /path/to/geant4/install/bin/geant4.sh

cd He3BeamSim
mkdir build && cd build
cmake ..
make -j$(nproc)
```

---

## Run

### Batch mode (100 000 events)

```bash
cd build
./He3BeamSim ../run.mac
```

Console output:

```
============================================
  He-3 Beam Broadening — Results
============================================
  Events: 100000   Hits: 99847

  Beam profile at scorer (z = 500 mm)
  ------------------------------------------
  Mean  (x,y)       : (-0.001, 0.002) mm
  Sigma_x           : 5.955 mm
  Sigma_y           : 5.948 mm
  FWHM_x            : 14.021 mm
  FWHM_y            : 14.005 mm
  Mean radius       : 6.624 mm
  RMS  radius       : 8.494 mm
  90% containment R : 9.412 mm
  95% containment R : 10.871 mm
  ------------------------------------------
  Mean KE at scorer : 5.839 MeV
  Energy loss       : 0.161 MeV
============================================
```

Output files written to the `build/` directory:
- `He3BeamSim.root` — ROOT file with histograms and TTree
- `plot_xy.png` — 2D transverse beam profile
- `plot_r.png` — radial distribution
- `plot_x.png`, `plot_y.png` — x and y projections
- `plot_energy.png` — kinetic energy distribution at scorer
- `hits_scoring_plane.csv` — per-hit (x, y, r, E) data

### Interactive / visualisation mode

```bash
# Opens Qt window with geometry pre-loaded
./He3BeamSim

# Runs vis.mac then keeps window open for interactive commands
./He3BeamSim ../vis.mac
```

In the interactive terminal, type Geant4 commands such as:
```
/run/beamOn 500
/vis/viewer/set/viewpointThetaPhi 90 0
exit
```

---

## Post-processing

```bash
# Python plots from CSV (no ROOT needed)
python3 ../plot_beam_profile.py hits_scoring_plane.csv

# Python plots from ROOT file
python3 ../plot_beam_profile.py He3BeamSim.root
```

---

## Physics notes

### Geometry

| Component      | Material    | Dimensions                  | Position        |
|----------------|-------------|-----------------------------|-----------------|
| World          | Vacuum      | radius = max(PipeD, ScorerD)/2 + 1 cm | — |
| Si₃N₄ foil    | Si₃N₄       | diameter = FoilD, thickness = FoilT | z = 0 |
| He-4 gas tube  | He-4 at GasPres | diameter = PipeD, length = Dist | z = 0 → Dist |
| Scoring plane  | Si (thin)   | diameter = ScorerD, thickness = 1 µm | z = Dist |

### He-4 gas density

The gas density is computed from the ideal gas law at 293 K:

```
ρ = P × M / (R × T)
  = GasPres[Torr] × 133.322 [Pa/Torr] × 4.003×10⁻³ [kg/mol]
    / (8.314 [J/mol·K] × 293.15 [K])
```

At 5 Torr: ρ ≈ 1.09×10⁻⁶ g/cm³. The gas material is built from the
NIST `G4_He` base (which carries full stopping power tables) scaled to
this density using `G4NistManager::BuildMaterialWithNewDensity`.

### Physics list

`QGSP_BIC` + `G4EmStandardPhysics_option4`:

| Component                     | Relevance                                  |
|-------------------------------|--------------------------------------------|
| `G4EmStandardPhysics_option4` | Urban MSC — best accuracy for ions         |
| `G4IonPhysics`                | He-3 nuclear interactions                  |
| `G4IonElasticPhysics`         | Elastic He-3 scattering                    |
| `G4HadronPhysicsQGSP_BIC`     | Hadronic inelastic                         |
| `G4StepLimiterPhysics`        | Enforces max step in gas volume            |

### Step limiter in gas

A `G4UserLimits` max step of `Dist/500` is applied inside the He-4 gas
volume. Without this, Geant4 takes a single giant step through the
low-density gas and the energy loss rounds to zero.

---

## Geometry sanity checks

The simulation prints warnings at startup if:
- `BeamD > FoilD` — beam extends beyond the foil (produces a hollow ring)
- `PipeD < BeamD` — gas tube is narrower than the beam

---

## Tuning suggestions

| Goal | Change in `sim.conf` |
|------|----------------------|
| Different beam size | `BeamD` |
| Thicker/thinner foil | `FoilT` (µm) |
| Higher gas pressure | `GasPres` (Torr) |
| Longer drift | `Dist` (mm) |
| Wider scorer | `ScorerD` (mm) |
| Narrower gas pipe | `PipeD` (mm) |