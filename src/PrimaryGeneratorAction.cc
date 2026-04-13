#include "PrimaryGeneratorAction.hh"
#include "SimConfig.hh"
#include "G4ParticleGun.hh"
#include "G4IonTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"
#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction()
  : G4VUserPrimaryGeneratorAction(),
    fParticleGun(new G4ParticleGun(1)),
    fBeamRadius((SimConfig::Instance().BeamD() / 2.0) * mm)
{
    auto* ion = G4IonTable::GetIonTable()->GetIon(2, 3, 0.0);
    fParticleGun->SetParticleDefinition(ion);
    fParticleGun->SetParticleCharge(2);
    fParticleGun->SetParticleEnergy(6.0*MeV);
    fParticleGun->SetParticleMomentumDirection(G4ThreeVector(0, 0, 1));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() { delete fParticleGun; }

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
    G4double r   = fBeamRadius * std::sqrt(G4UniformRand());
    G4double phi = twopi * G4UniformRand();
    fParticleGun->SetParticlePosition(
        G4ThreeVector(r*std::cos(phi), r*std::sin(phi), -1.0*mm));
    fParticleGun->GeneratePrimaryVertex(event);
}
