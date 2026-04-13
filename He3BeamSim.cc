#include "DetectorConstruction.hh"
#include "ActionInitialization.hh"
#include "SimConfig.hh"
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "G4UIExecutive.hh"
#include "G4VisExecutive.hh"
#include "G4SystemOfUnits.hh"
#include "QGSP_BIC.hh"
#include "G4EmStandardPhysics_option4.hh"
#include "G4StepLimiterPhysics.hh"
#include "G4EmParameters.hh"
#include <string>

int main(int argc, char** argv)
{
    SimConfig::Instance().Load("../sim.conf");

    auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Serial);
    auto* detector = new DetectorConstruction();
    runManager->SetUserInitialization(detector);

    auto* physicsList = new QGSP_BIC();
    physicsList->ReplacePhysics(new G4EmStandardPhysics_option4());
    auto* stepLim = new G4StepLimiterPhysics();
    stepLim->SetApplyToAll(true);
    physicsList->RegisterPhysics(stepLim);
    runManager->SetUserInitialization(physicsList);
    runManager->SetUserInitialization(new ActionInitialization(detector));

    G4EmParameters::Instance()->SetMinEnergy(100*eV);
    G4EmParameters::Instance()->SetApplyCuts(false);

    auto* UImanager = G4UImanager::GetUIpointer();

    if (argc > 1) {
        std::string macro(argv[1]);
        bool isVis = (macro.find("vis") != std::string::npos);
        if (isVis) {
            auto* vis = new G4VisExecutive("Quiet");
            vis->Initialize();
            auto* ui = new G4UIExecutive(argc, argv);
            UImanager->ApplyCommand("/control/execute " + macro);
            ui->SessionStart();
            delete ui;
            delete vis;
        } else {
            UImanager->ApplyCommand("/control/execute " + macro);
        }
    } else {
        auto* vis = new G4VisExecutive("Quiet");
        vis->Initialize();
        auto* ui = new G4UIExecutive(argc, argv);
        UImanager->ApplyCommand("/control/execute ../init_vis.mac");
        ui->SessionStart();
        delete ui;
        delete vis;
    }

    delete runManager;
    return 0;
}
