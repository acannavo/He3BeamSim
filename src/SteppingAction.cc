#include "SteppingAction.hh"
#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"

SteppingAction::SteppingAction(const DetectorConstruction* det, EventAction* ev)
  : G4UserSteppingAction(), fDetector(det), fEventAction(ev) {}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
    // Only primary
    if (step->GetTrack()->GetTrackID() != 1) return;

    // Get volumes FIRST
    auto* preVol  = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
    auto* postVol = step->GetPostStepPoint()->GetTouchableHandle()->GetVolume();

    if (!preVol || !postVol) return;

    auto* preLV  = preVol->GetLogicalVolume();
    auto* postLV = postVol->GetLogicalVolume();

    auto* gasLV = fDetector->GetGasVolume();

    
    auto stepStatus = step->GetPostStepPoint()->GetStepStatus();

    if (stepStatus == fGeomBoundary) {

        // --- ENTRY into gas ---
        if (postLV == gasLV && preLV != gasLV) {
            G4double ekin = step->GetPostStepPoint()->GetKineticEnergy();
            fEventAction->SetEInGas(ekin);

            // G4cout << ">>> ENTER GAS, E = " << ekin/MeV << " MeV\n";
        }

        // --- EXIT from gas ---
        if (preLV == gasLV && postLV != gasLV) {
            G4double ekin = step->GetPreStepPoint()->GetKineticEnergy();
            fEventAction->SetEOutGas(ekin);

            // G4cout << "<<< EXIT GAS, E = " << ekin/MeV << " MeV\n";
        }
    }
    // =========================================================

    // --- SCORER (keep this LAST) ---
    if (postLV == fDetector->GetScoringVolume()) {
        G4ThreeVector pos  = step->GetPostStepPoint()->GetPosition();
        G4double      ekin = step->GetPostStepPoint()->GetKineticEnergy();

        fEventAction->AddHit(pos.x(), pos.y(), ekin);
        step->GetTrack()->SetTrackStatus(fStopAndKill);
    }
}