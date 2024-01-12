#ifndef FocusingGeometry_h
#define FocusingGeometry_h

#include <vector>

#include "TObject.h"

#include "Horn.h"
#include "Particle.h"

class FocusingGeometry : public TObject {
  public:
    FocusingGeometry();
    FocusingGeometry(Horn * h1, Horn * h2, std::vector<double> b);
    ~FocusingGeometry();
    
    void PropagateParticle(Particle * p);
    void IdealFocus(Particle * p);
    void NoFocus(Particle * p);
    
    Double_t GetNumTrajPoints() { return numPoints; };
    Double_t GetZStep() { return zStep; };
    Double_t GetYLimit() { return yLimitMax; };
    
    void SetNumTrajPoints(Double_t a) { numPoints = a; };
    void SetZStep(Double_t a) { zStep = a; };
    void SetYLimit(Double_t a) { yLimitMax = a; };
    void SetVerbosityLevel(int a) { VerbosityLevel = a;};
    
    void SetCheckpointZ(Double_t d) { checkpointZ = d;}
    Double_t GetCheckpointZ() { return checkpointZ; };
    Double_t GetCheckpointY() { return checkpointY; };
    
    std::vector<Double_t> GetTrajX() { return trajX; };
    std::vector<Double_t> GetTrajY() { return trajY; };
    std::vector<Double_t> GetTrajZ() { return trajZ; };

  private:
    // variables
    Double_t numPoints, zStep, yLimitMax;
    int VerbosityLevel;
    Double_t checkpointZ;
    Double_t checkpointY;
    
    Horn *h1_;
    Horn *h2_;
    std::vector<double> baselines;
    
    std::vector<Double_t> trajX;
    std::vector<Double_t> trajY;
    std::vector<Double_t> trajZ;
    
    // methods
    void UpdateHorn(Horn * h, Double_t & yCurr, Double_t & zCurr, Double_t & alpha, Double_t & delta, const Double_t bHorn, const Double_t angleCurr );
    bool initialized;
    
  ClassDef(FocusingGeometry,1)
};

#endif