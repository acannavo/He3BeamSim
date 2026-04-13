#include "EventAction.hh"
#include "RunAction.hh"
#include "G4SystemOfUnits.hh"

EventAction::EventAction(RunAction* ra)
  : G4UserEventAction(),
    fRunAction(ra),
    fHitRecorded(false)
{}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*)
{
    fHitRecorded = false;

    fEInGas = -1.0;
    fEOutGas = -1.0;
    fEnteredGas = false;
    fExitedGas  = false;
}

void EventAction::EndOfEventAction(const G4Event*)
{
    if (fEnteredGas && fExitedGas) {
        G4double dE = fEInGas - fEOutGas;
        fRunAction->RecordDeltaE(dE);
    }
}

void EventAction::AddHit(G4double x, G4double y, G4double ekin)
{
    if (fHitRecorded) return;
    fHitRecorded = true;
    fRunAction->RecordHit(x, y, ekin);
}

// NEW
void EventAction::SetEInGas(G4double e)
{
    if (!fEnteredGas) {
        fEInGas = e;
        fEnteredGas = true;
    }
}

void EventAction::SetEOutGas(G4double e)
{
    if (!fExitedGas) {
        fEOutGas = e;
        fExitedGas = true;
    }
}