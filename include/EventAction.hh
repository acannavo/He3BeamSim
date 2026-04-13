#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class RunAction;

class EventAction : public G4UserEventAction
{
public:
    EventAction(RunAction* ra);
    virtual ~EventAction();

    virtual void BeginOfEventAction(const G4Event*);
    virtual void EndOfEventAction(const G4Event*);

    void AddHit(G4double x, G4double y, G4double ekin);

    // NEW
    void SetEInGas(G4double e);
    void SetEOutGas(G4double e);

private:
    RunAction* fRunAction;

    G4bool fHitRecorded;

    // NEW
    G4double fEInGas;
    G4double fEOutGas;
    G4bool   fEnteredGas;
    G4bool   fExitedGas;
};

#endif