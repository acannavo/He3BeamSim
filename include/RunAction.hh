#ifndef RunAction_h
#define RunAction_h 1
#include "G4UserRunAction.hh"
#include "globals.hh"
#include <vector>
class G4Run;

class RunAction : public G4UserRunAction
{
public:
    RunAction();
    ~RunAction() override;
    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction  (const G4Run*) override;

    // Called by EventAction for each scorer hit
    void RecordHit(G4double x, G4double y, G4double ekin);
	
	void RecordDeltaE(G4double dE);

private:
    std::vector<G4double> fHitX;
    std::vector<G4double> fHitY;
    std::vector<G4double> fHitE;   // kinetic energy at scorer [MeV]
};
#endif
