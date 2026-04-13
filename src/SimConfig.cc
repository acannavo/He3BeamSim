#include "SimConfig.hh"
#include <fstream>
#include <iostream>

SimConfig& SimConfig::Instance()
{
    static SimConfig inst;
    return inst;
}
SimConfig::SimConfig() {}

static std::string clean(std::string s)
{
    auto c = s.find('#');
    if (c != std::string::npos) s = s.substr(0, c);
    auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string::npos) return "";
    auto e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

void SimConfig::Load(const std::string& filename)
{
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "[SimConfig] WARNING: cannot open '" << filename
                  << "' — using defaults.\n";
        return;
    }
    std::string line;
    while (std::getline(f, line)) {
        line = clean(line);
        if (line.empty()) continue;
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = clean(line.substr(0, eq));
        std::string val = clean(line.substr(eq + 1));
        if (key.empty() || val.empty()) continue;
        double v = std::stod(val);
        if      (key == "BeamD")   fBeamD   = v;
        else if (key == "FoilD")   fFoilD   = v;
        else if (key == "FoilT")   fFoilT   = v;
        else if (key == "PipeD")   fPipeD   = v;
        else if (key == "Dist")    fDist    = v;
        else if (key == "ScorerD") fScorerD = v;
        else if (key == "GasPres") fGasPres = v;
        else std::cerr << "[SimConfig] Unknown key: '" << key << "'\n";
    }
    std::cout << "\n[SimConfig] Loaded '" << filename << "':\n"
              << "  BeamD   = " << fBeamD   << " mm\n"
              << "  FoilD   = " << fFoilD   << " mm\n"
              << "  FoilT   = " << fFoilT   << " um\n"
              << "  PipeD   = " << fPipeD   << " mm\n"
              << "  Dist    = " << fDist    << " mm\n"
              << "  ScorerD = " << fScorerD << " mm\n"
              << "  GasPres = " << fGasPres << " Torr\n\n";
}
