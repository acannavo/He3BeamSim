#include "ActionInitialization.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "EventAction.hh"
#include "SteppingAction.hh"
#include "DetectorConstruction.hh"

ActionInitialization::ActionInitialization(DetectorConstruction* det)
  : G4VUserActionInitialization(), fDetector(det) {}

ActionInitialization::~ActionInitialization() {}

void ActionInitialization::BuildForMaster() const
{
    SetUserAction(new RunAction());
}

void ActionInitialization::Build() const
{
    SetUserAction(new PrimaryGeneratorAction());
    auto* run   = new RunAction();
    SetUserAction(run);
    auto* event = new EventAction(run);
    SetUserAction(event);
    SetUserAction(new SteppingAction(fDetector, event));
}
