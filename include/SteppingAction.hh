#ifndef SteppingAction_h
#define SteppingAction_h 1
#include "G4UserSteppingAction.hh"
class DetectorConstruction;
class EventAction;
class SteppingAction : public G4UserSteppingAction
{
public:
    SteppingAction(const DetectorConstruction* det, EventAction* ev);
    ~SteppingAction() override;
    void UserSteppingAction(const G4Step*) override;
private:
    const DetectorConstruction* fDetector;
    EventAction*                fEventAction;
};
#endif
