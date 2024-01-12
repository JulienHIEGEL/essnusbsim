#ifndef CngsTarget_h
#define CngsTarget_h

#include <map>
#include <vector>

#include <TChain.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TObject.h>
#include <TROOT.h>

// #include "FocusingGeometry.h"
// #include "Horn.h"
#include "Particle.h"

// From HELAS
// This subroutine performs the spacial rotation of a four-momentum.
// the momentum p is assumed to be given in the frame where the spacial
// component of q points the positive z-axis.  prot is the momentum p
// rotated to the frame where q is given.
extern "C" void rotxxx_(double*,double*,double*);

// used in Vegas intergration
struct Interval {
  double xLow;
  double xHigh;
  double Length() 
  {
    return xHigh - xLow;
  }
  
  // grid[i] = location of subdivision #i of the interval,
  // NORMALIZED TO LIE BETWEEN 0 AND 1.
  std::vector<double> grid;
  
  void Rebin(double rc, int ndiv, std::vector<double> r);
};

// Four-vector, used in three-body decay
class FourVector {
public:
  double & E() {
    return P[0];
  }
  double & Px() { 
    return P[1];
  }
  double & Py() {
    return P[2];
  }
  double & Pz() {
    return P[3];
  }
  double PSpace() {
    return sqrt(P[1]*P[1]+P[2]*P[2]+P[3]*P[3]);
  }
  double M() {
    double pp = PSpace();
    double msq = P[0]*P[0]-pp*pp;
    if(msq < 0. && msq > -1.e-6)
      msq = 0.;
    // if it's otherwise tachyonic, throw an exception?
      return sqrt(msq);
  }
  
  // this is accessible directly, for ease
  // of calling Fortran code
  double P[4];
};

class CngsTarget {
public :
  
  // I/O
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain
   
  TFile *hFile;        // output file

  // Declaration of leaf types
  Particle        *p;
  
  // Auxiliary variables
  std::string inputFile;
  std::map<unsigned int, unsigned long int> particleContent;
  
  struct DecayProduct 
  {
    Double_t thetaLab;
    Double_t thetaParent;
    Double_t momentum;
    Double_t genPos;
    bool decayed;
    
    DecayProduct(Double_t tl, Double_t tp, Double_t m, Double_t p) : 
    thetaLab(tl),thetaParent(tp), momentum(m), genPos(p)
      {decayed = false;}
  };
  
  std::vector<DecayProduct> nu_mus;
  std::vector<DecayProduct> nu_es;
  std::vector<DecayProduct> mus;
  std::vector<DecayProduct> es;
  
  // List of histograms
  TH1D           *h_particleContent;
  TH1D           *h_testRand;
  std::map<std::string, TH1*> h1_;
  std::map<std::string, TH2*> h2_;
  
  CngsTarget(std::string inputFile);
  virtual ~CngsTarget();
  virtual Int_t    GetEntry(Long64_t entry);
  virtual Long64_t LoadTree(Long64_t entry);
  virtual void     Init(TTree *tree);
  virtual void     Loop(Long64_t limitEvents);
  virtual Bool_t   Notify();
  
  // Analysis-related methods
  void BookHistograms();
  void FillHistograms();
  void NeutrinoFluxHistograms(std::string region, unsigned int which);
  void MuonFluxHistograms(std::string region, unsigned int which);
  void GeneratePionDaughters();
  void GenerateKaonDaughters();
  Double_t LabAngle(Double_t beta, Double_t cosTheta);
  Double_t NeutrinoEnergy(Double_t pionEnergy, Double_t particleMass, Double_t neutrinoLabAngle);
  Double_t MuonEnergy(Double_t pionEnergy, Double_t particleMass, Double_t muonLabAngle);
  void MuonTransportAndDecay(unsigned int which, Double_t decayPipeLength);
  bool CrossesNearDetector(Double_t detectorPos, Double_t decayPos, Double_t thetaLab);
  void BoostFourMom(FourVector p, FourVector q, FourVector & pboost);
  
  // Three-body decay
  void VegasThreeBodyDecay(unsigned int which, Double_t pos);
  void ThreeBodyDecay(std::vector<double> x, unsigned int which, Double_t pos);
  enum SampleMethod {IMPORT, STRAT_MAG, STRAT_ERR};
  typedef std::vector<Interval> Region;
  
  // Auxiliary methods
  long int GetTimeLeft(double PercentDone, double SecondsTaken);
  void MkCdDir(std::string root, std::string newdir);
};





#endif
