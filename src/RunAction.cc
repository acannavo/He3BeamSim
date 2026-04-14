#include "RunAction.hh"
#include "SimConfig.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnitsTable.hh"
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"
#include "RunAction.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"


RunAction::RunAction() : G4UserRunAction() {}
RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{
    fHitX.clear(); fHitY.clear(); fHitE.clear();
    fHitX.reserve(100000); fHitY.reserve(100000); fHitE.reserve(100000);
}

void RunAction::RecordHit(G4double x, G4double y, G4double ekin)
{
    fHitX.push_back(x);
    fHitY.push_back(y);
    fHitE.push_back(ekin);
}

void RunAction::RecordDeltaE(G4double dE)
{
    //G4cout << "DeltaE (keV): " << dE/keV << G4endl;
}

void RunAction::EndOfRunAction(const G4Run* run)
{
    G4int nEvents = run->GetNumberOfEvent();
    G4int nHits   = (G4int)fHitX.size();
    if (nHits == 0) { G4cout << "No hits!\n"; return; }

    const SimConfig& cfg = SimConfig::Instance();
    double pipeR_mm = cfg.PipeD() / 2.0;

    G4double sumX=0,sumX2=0,sumY=0,sumY2=0,sumE=0,sumE2=0;
    std::vector<G4double> rv(nHits);
    for (int i=0;i<nHits;i++){
        sumX  += fHitX[i]; sumX2 += fHitX[i]*fHitX[i];
        sumY  += fHitY[i]; sumY2 += fHitY[i]*fHitY[i];
        sumE  += fHitE[i]; sumE2 += fHitE[i]*fHitE[i];
        rv[i]  = std::sqrt(fHitX[i]*fHitX[i]+fHitY[i]*fHitY[i]);
    }
    G4double meanX  = sumX/nHits, meanY=sumY/nHits;
    G4double sigmaX = std::sqrt(sumX2/nHits - meanX*meanX);
    G4double sigmaY = std::sqrt(sumY2/nHits - meanY*meanY);
    G4double meanE  = sumE/nHits;
    G4double sigmaE = std::sqrt(sumE2/nHits - meanE*meanE);
    G4double meanR  = std::accumulate(rv.begin(),rv.end(),0.0)/nHits;
    G4double rmsR   = std::sqrt((sumX2+sumY2)/nHits);
    std::sort(rv.begin(),rv.end());
    G4double r90 = rv[(int)(0.90*nHits)];
    G4double r95 = rv[(int)(0.95*nHits)];
    const G4double k = 2.0*std::sqrt(2.0*std::log(2.0));

    G4cout << "\n============================================\n";
    G4cout << "  He-3 Beam Broadening — Results\n";
    G4cout << "============================================\n";
    G4cout << "  Events: " << nEvents << "   Hits: " << nHits << "\n\n";
    G4cout << std::fixed << std::setprecision(3);
    G4cout << "  Beam profile at scorer (z = " << cfg.Dist() << " mm)\n";
    G4cout << "  ------------------------------------------\n";
    G4cout << "  Mean  (x,y)       : (" << meanX/mm << ", " << meanY/mm << ") mm\n";
    G4cout << "  Sigma_x           : " << sigmaX/mm  << " mm\n";
    G4cout << "  Sigma_y           : " << sigmaY/mm  << " mm\n";
    G4cout << "  FWHM_x            : " << k*sigmaX/mm << " mm\n";
    G4cout << "  FWHM_y            : " << k*sigmaY/mm << " mm\n";
    G4cout << "  Mean radius       : " << meanR/mm    << " mm\n";
    G4cout << "  RMS  radius       : " << rmsR/mm     << " mm\n";
    G4cout << "  90% containment R : " << r90/mm      << " mm\n";
    G4cout << "  95% containment R : " << r95/mm      << " mm\n";
    G4cout << "  ------------------------------------------\n";
    G4cout << "  Mean KE at scorer : " << meanE/MeV   << " MeV\n";
    G4cout << "  Sigma KE          : " << sigmaE/MeV  << " MeV\n";
    G4cout << "  Energy loss       : " << (6.0 - meanE/MeV) << " MeV\n";
    G4cout << "============================================\n\n";

    // CSV
    {
        std::ofstream csv("hits_scoring_plane.csv");
        csv << "x_mm,y_mm,r_mm,ekin_MeV\n";
        for (int i=0;i<nHits;i++)
            csv << fHitX[i]/mm << "," << fHitY[i]/mm << ","
                << rv[i]/mm    << "," << fHitE[i]/MeV << "\n";
        G4cout << "  CSV: hits_scoring_plane.csv\n";
    }

    // ROOT
    gROOT->SetBatch(kTRUE);
    gStyle->SetOptStat(1111);
    gStyle->SetPalette(kBird);
    TFile* rootFile = TFile::Open("He3BeamSim.root","RECREATE");
    double rMax=pipeR_mm*1.05, xyMax=pipeR_mm*1.05;
    double eMin=std::max(0.0,meanE/MeV-5*sigmaE/MeV), eMax=meanE/MeV+5*sigmaE/MeV;
    TH1D* h_r =new TH1D("h_r","Radial;r[mm];Counts",200,0,rMax);
    TH1D* h_x =new TH1D("h_x","x;x[mm];Counts",200,-xyMax,xyMax);
    TH1D* h_y =new TH1D("h_y","y;y[mm];Counts",200,-xyMax,xyMax);
    TH2D* h_xy=new TH2D("h_xy","XY;x[mm];y[mm]",200,-xyMax,xyMax,200,-xyMax,xyMax);
    TH1D* h_e =new TH1D("h_e","Energy;E[MeV];Counts",200,eMin,eMax);
    TTree* tree=new TTree("hits","hits");
    double tx,ty,tr,te;
    tree->Branch("x",&tx,"x/D"); tree->Branch("y",&ty,"y/D");
    tree->Branch("r",&tr,"r/D"); tree->Branch("ekin",&te,"ekin/D");
    for (int i=0;i<nHits;i++){
        double xm=fHitX[i]/mm,ym=fHitY[i]/mm,rm=rv[i]/mm,em=fHitE[i]/MeV;
        h_r->Fill(rm); h_x->Fill(xm); h_y->Fill(ym); h_xy->Fill(xm,ym); h_e->Fill(em);
        tx=xm; ty=ym; tr=rm; te=em; tree->Fill();
    }
    {TCanvas c("c_xy","",800,700); h_xy->Draw("COLZ"); c.SaveAs("plot_xy.png");}
    {TCanvas c("c_r","",800,600);  h_r->Draw("HIST");  c.SaveAs("plot_r.png"); }
    {TCanvas c("c_x","",800,600);  h_x->Draw("HIST");  c.SaveAs("plot_x.png"); }
    {TCanvas c("c_y","",800,600);  h_y->Draw("HIST");  c.SaveAs("plot_y.png"); }
    {TCanvas c("c_e","",800,600);  h_e->Draw("HIST");  c.SaveAs("plot_energy.png");}
    rootFile->Write(); rootFile->Close();
    G4cout << "  ROOT: He3BeamSim.root  PNG: plot_*.png\n\n";
}
