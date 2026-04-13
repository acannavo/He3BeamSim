#include "DetectorConstruction.hh"
#include "SimConfig.hh"
#include "G4NistManager.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4UserLimits.hh"
#include <algorithm>

DetectorConstruction::DetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
    const SimConfig& cfg = SimConfig::Instance();

    const G4double foilR   = (cfg.FoilD()   / 2.0) * mm;
    const G4double pipeR   = (cfg.PipeD()   / 2.0) * mm;
    const G4double scorerR = (cfg.ScorerD() / 2.0) * mm;
    const G4double foilHz  = (cfg.FoilT()   / 2.0) * um;
    const G4double drift   =  cfg.Dist()            * mm;

    const G4double scoreHz = 0.5 * um;
    const G4double gasHz   = drift / 2.0;
    const G4double gasZ    = foilHz + gasHz;
    const G4double scoreZ  = foilHz + drift + scoreHz;
    const G4double worldR  = std::max({pipeR, scorerR, foilR}) + 1.0*cm;
    const G4double worldHz = scoreZ + 5.0*cm;

    G4NistManager* nist = G4NistManager::Instance();

    // World vacuum
    G4Material* vacuum = new G4Material("Vacuum", 1.e-10*g/cm3, 1,
                                         kStateGas, 293.15*kelvin, 3.e-18*pascal);
    vacuum->AddMaterial(nist->FindOrBuildMaterial("G4_N"), 1.0);

    // Si3N4
    G4Element* Si = nist->FindOrBuildElement("Si");
    G4Element* N  = nist->FindOrBuildElement("N");
    G4Material* Si3N4 = new G4Material("Si3N4", 3.17*g/cm3, 2);
    Si3N4->AddElement(Si, 3);
    Si3N4->AddElement(N,  4);

    // He-4 gas using BuildMaterialWithNewDensity — this is the NIST-recommended
    // way to get a gas at non-standard pressure. It inherits all ICRU stopping
    // power tables from G4_He but uses our pressure-derived density.
    const G4double He4press_Torr = cfg.GasPres();
    const G4double He4press_Pa   = He4press_Torr * 133.322;
    const G4double He4temp_K     = 293.15;
    const G4double He4density    = (He4press_Pa * 4.002602e-3)
                                  / (8.314 * He4temp_K)
                                  / 1000.0;   // g/cm3

    // BuildMaterialWithNewDensity(name, base_nist_name, density, temp, pressure)
    G4Material* He4gas = nist->BuildMaterialWithNewDensity(
        "He4gas",          // new name
        "G4_He",           // base NIST material (has full ICRU stopping data)
        He4density,        // our density in g/cm3
        He4temp_K*kelvin,  // temperature
        He4press_Pa*pascal // pressure
    );

    G4cout << "\n[Geometry] He-4 gas (built from G4_He via NIST):\n"
           << "  Pressure : " << He4press_Torr << " Torr (" << He4press_Pa << " Pa)\n"
           << "  Temp     : " << He4temp_K << " K\n"
           << "  Density  : " << He4density << " g/cm3\n\n";

    G4Material* silicon = nist->FindOrBuildMaterial("G4_Si");

    // World
    auto* solidWorld = new G4Tubs("World", 0, worldR, worldHz, 0, 360*deg);
    auto* logicWorld = new G4LogicalVolume(solidWorld, vacuum, "World");
    auto* physWorld  = new G4PVPlacement(nullptr, G4ThreeVector(),
                                          logicWorld, "World",
                                          nullptr, false, 0, true);
    logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());

    // Si3N4 foil
    auto* solidFoil = new G4Tubs("Si3N4Win", 0, foilR, foilHz, 0, 360*deg);
    auto* logicFoil = new G4LogicalVolume(solidFoil, Si3N4, "Si3N4Win");
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, 0),
                      logicFoil, "Si3N4Win", logicWorld, false, 0, true);
    auto* foilVis = new G4VisAttributes(G4Colour(0.0, 0.8, 0.4, 0.7));
    foilVis->SetForceSolid(true);
    logicFoil->SetVisAttributes(foilVis);

    // He-4 gas cylinder
    auto* solidGas = new G4Tubs("He4Gas", 0, pipeR, gasHz, 0, 360*deg);
    auto* logicGas = new G4LogicalVolume(solidGas, He4gas, "He4Gas");
    G4double maxStep = drift / 500.0;
    logicGas->SetUserLimits(new G4UserLimits(maxStep));
	fGasVolume = logicGas;   
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, gasZ),
                      logicGas, "He4Gas", logicWorld, false, 0, true);
    auto* gasVis = new G4VisAttributes(G4Colour(0.5, 0.8, 1.0, 0.15));
    gasVis->SetForceSolid(true);
    logicGas->SetVisAttributes(gasVis);

    // Scorer
    auto* solidScore = new G4Tubs("Scorer", 0, scorerR, scoreHz, 0, 360*deg);
    auto* logicScore = new G4LogicalVolume(solidScore, silicon, "Scorer");
    new G4PVPlacement(nullptr, G4ThreeVector(0, 0, scoreZ),
                      logicScore, "Scorer", logicWorld, false, 0, true);
    auto* scoreVis = new G4VisAttributes(G4Colour(0.2, 0.4, 1.0, 0.5));
    scoreVis->SetForceSolid(true);
    logicScore->SetVisAttributes(scoreVis);

    if (cfg.BeamD() > cfg.FoilD())
        G4cerr << "\n[WARNING] BeamD > FoilD!\n\n";
    if (cfg.PipeD() < cfg.BeamD())
        G4cerr << "\n[WARNING] PipeD < BeamD!\n\n";

    G4cout << "[Geometry] Layout:\n"
           << "  Foil    : z = 0,              R = " << foilR/mm   << " mm\n"
           << "  He4 gas : z = " << gasZ/mm   << " mm (centre), R = " << pipeR/mm   << " mm\n"
           << "  Scorer  : z = " << scoreZ/mm << " mm,          R = " << scorerR/mm << " mm\n"
           << "  Max step in gas: " << maxStep/mm << " mm\n\n";

    fScoringVolume = logicScore;
    return physWorld;
}
