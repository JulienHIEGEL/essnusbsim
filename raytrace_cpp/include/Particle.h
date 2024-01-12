// Generic class for pion transport and decay

// v3 : Adding neutrino decay info
// v4 : Removing trajectory
// v5 : Modified neutrino decay info

#ifndef __particle__
#define __particle__

#include <cmath>

#include "TObject.h"

class Particle : public TObject {
  public:  
    Particle();
    Particle(Double_t ev_, UInt_t i, Double_t x_, Double_t y_, Double_t z_, Double_t px_, Double_t py_, Double_t pz_);
    ~Particle();
    
    Double_t P() { return sqrt(py*py + pz*pz); };
    Double_t Pt() { return sqrt(py*py); };// SetPt_Out(p->py); in NoFocus!
    Double_t Pl() { return pz; };
    
    Double_t ThetaIn() { return py>0?1000*atan (Pt()/Pl()):-1000*atan (Pt()/Pl()); }; // in mrad
    Double_t ThetaOut() { return pT_out>0?1000*atan (pT_out/pL_out):-1000*atan (pT_out/pL_out); };
    
    void SetEventNum(Double_t d) { event = d; };
    void SetPt_Out(Double_t d) { pT_out = d; };
    void SetPl_Out(Double_t d) { pL_out = d; };
    void SetXOut(Double_t d) { xOut = d; };
    void SetYOut(Double_t d) { yOut = d; };
    void SetZOut(Double_t d) { zOut = d; };
    
    void SetDecayPos(Double_t p) { decayPos = p; }
    
    Double_t Pt_Out() { return pT_out; };
    Double_t Pl_Out() { return pL_out; };
    
    Double_t GetEventNum() { return event; }
    Double_t GetDecayPos() { return decayPos; }
    Double_t GetXOut() { return xOut; }
    Double_t GetYOut() { return yOut; }
    Double_t GetZOut() { return zOut; }
 
    void SetCheckpointZ(Double_t d) { checkpointZ = d;}
    Double_t GetCheckpointZ() { return checkpointZ; };
    void SetCheckpointY(Double_t d) { checkpointY = d;}
    Double_t GetCheckpointY() { return checkpointY; };
    
    
    Double_t event;
    UInt_t   id;
    Double_t x;
    Double_t y;
    Double_t z;
    Double_t px;
    Double_t py;
    Double_t pz;
    Double_t mass;
    Double_t energy;
    
  private:
    Double_t yOut;
    Double_t xOut;
    Double_t zOut;
    Double_t pT_out; 
    Double_t pL_out;
    Double_t decayPos;
    Double_t checkpointZ;
    Double_t checkpointY;
    
    ClassDef(Particle,1)
};

#endif
