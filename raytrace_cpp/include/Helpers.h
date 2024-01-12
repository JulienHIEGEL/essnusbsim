#ifndef MYFUNCTS_H_
#define MYFUNCTS_H_

#include <cstdlib>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "TH1.h"
#include "TH2D.h"
#include "TH2F.h"

namespace helpers
{
  enum FlukaParticleCodes
  {
    PROTON   = 1,
    APROTON  = 2,
    ELECTRON = 3,
    POSITRON = 4,
    NEUTRIE  = 5,
    ANEUTRIE = 6,
    PHOTON   = 7,
    NEUTRON  = 8,
    ANEUTRON = 9,
    MUON_P   = 10,
    MUON_M   = 11,
    KAON_L   = 12,
    PION_P   = 13,
    PION_M   = 14,
    KAON_P   = 15,
    KAON_M   = 16,
    LAMBDA   = 17,
    ALAMBDA  = 18,
    KAON_S   = 19,
    SIGMA_M  = 20,
    SIGMA_P  = 21,
    SIGMA_0  = 22, 
    PI_0     = 23,
    KAON_0   = 24,
    AKAON_0  = 25,
    NEUTRIM  = 27,
    ANEUTRIM = 28,
    TAU_P    = 41,
    TAU_M    = 42,
    NEUTRIT  = 43,
    ANEUTRIT = 44
  };
  
  struct hBins
  {
      std::string yAxis;
      std::string xAxis;
      int nBins;
      double low;
      double high;
  };
  
  // calculating dPhi
  inline double deltaPhi(double phi1, double phi2) 
  { 
      double result = phi1 - phi2;
      while (result > M_PI) result -= 2*M_PI;
      while (result <= -M_PI) result += 2*M_PI;
      return result;
  }

  // calculating deltaR
  inline double deltaR(double eta1, double phi1, double eta2, double phi2)
  {
      double deta = eta1 - eta2;
      double dphi = deltaPhi(phi1, phi2);
      return sqrt(deta*deta + dphi*dphi);
  }


  // convert int to string
  inline std::string itos(int i) 
  {
      std::stringstream s;
      s << i;
      return s.str();
  }


  // set integer bin labels to a given plot
  inline void setIntegerBinLabels( TH1* h )
  {
      h->GetXaxis()->SetNdivisions( h->GetNbinsX() );
      h->GetXaxis()->CenterLabels(true);
  }
  

  // set integer bin labels to a given 2d plot
  inline void setIntegerBinLabels2D( TH1* h )
  {
      h->GetXaxis()->SetNdivisions( h->GetNbinsX() );
      h->GetYaxis()->SetNdivisions( h->GetNbinsY() );
      h->GetXaxis()->CenterLabels(true);
      h->GetYaxis()->CenterLabels(true);
  }
  
  TH1D * myTH1D(std::string name,
                          hBins bins,
                          int lColor = kBlack,
                          int lWidth = 2,
                          int lStyle = 1,
                          int mStyle = 1 );

  TH1F * myTH1F(std::string name,
                          hBins bins,
                          int lColor = kBlack,
                          int lWidth = 2,
                          int lStyle = 1,
                          int mStyle = 1 );                          
                          
  TH1D * myTH1D(std::string name, 
                          hBins bins,
                          const double *xbins,
                          int lColor = kBlack,
                          int lWidth = 2,
                          int lStyle = 1,
                          int mStyle = 1 );
                          
  TH1F * myTH1F(std::string name, 
                          hBins bins,
                          const double *xbins,
                          int lColor = kBlack,
                          int lWidth = 2,
                          int lStyle = 1,
                          int mStyle = 1 );
    
  TH2D * myTH2D(std::string name, hBins binsX, hBins binsY );
  TH2F * myTH2F(std::string name, hBins binsX, hBins binsY );
  // used to format single-digit integers
  inline std::string CorrStr(int i){ if (i > 99) return itos(i); if (i > 9) return "0" + itos(i); else return "00" + itos(i); }
  
  std::vector<std::string> stringParser(std::string cutString);
//   std::vector<double> stringParserLong(std::string cutString);
  void stringParserLong(std::string cutString, std::vector<double>& toReturn);
  
  std::string flukaParticle(int particleCode);
  std::string pdgParticle(int particleCode);
  int flukaParticleIdToPythia(int particleCode);
}

#endif
