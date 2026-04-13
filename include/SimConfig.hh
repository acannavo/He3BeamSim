#ifndef SimConfig_h
#define SimConfig_h 1
#include <string>

class SimConfig
{
public:
    static SimConfig& Instance();
    void Load(const std::string& filename);

    double BeamD()   const { return fBeamD;   }   // mm
    double FoilD()   const { return fFoilD;   }   // mm
    double FoilT()   const { return fFoilT;   }   // um
    double PipeD()   const { return fPipeD;   }   // mm
    double Dist()    const { return fDist;    }   // mm
    double ScorerD() const { return fScorerD; }   // mm
    double GasPres() const { return fGasPres; }   // Torr

private:
    SimConfig();
    double fBeamD   =   20.0;
    double fFoilD   =   30.0;
    double fFoilT   =    1.0;
    double fPipeD   =   60.0;
    double fDist    =  500.0;
    double fScorerD =  200.0;
    double fGasPres =    5.0;
};
#endif
