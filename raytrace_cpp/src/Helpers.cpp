#include "../include/Helpers.h"
#include <iostream>
using namespace std;
#include "TH1D.h"

//_____________________________________________________________________________
std::vector<std::string>  helpers::stringParser(std::string cutString)
{
  std::vector<std::string> toReturn;
  std::stringstream s1;
  
  for(std::string::const_iterator itr=cutString.begin(); itr!=cutString.end(); itr++)
  {
    if((*itr)!=';')
      s1<<*itr;
    else
    {
      toReturn.push_back(s1.str());
      s1.str("");
    }
    
    if( itr==(cutString.end()-1))
    {
      toReturn.push_back(s1.str());
      s1.str("");
    }
  } 
  
  return toReturn;
}

//_____________________________________________________________________________
void  helpers::stringParserLong(std::string cutString, std::vector<double>& toReturn)
{
//   std::vector<double> toReturn;
  std::stringstream s1;
  
  for(std::string::const_iterator itr=cutString.begin(); itr!=cutString.end(); itr++)
  {
    if((*itr)!=';')
      s1<<*itr;
    else
    {
      toReturn.push_back(atof(s1.str().c_str()));
      s1.str("");
    }
    
    if( itr==(cutString.end()-1))
    {
      toReturn.push_back(atof(s1.str().c_str()));
      s1.str("");
    }
  } 
  
//   return toReturn;
}

TH1D * helpers::myTH1D(std::string name, hBins bins, int lColor, int lWidth, int lStyle, int mStyle )
{
    TH1D* h = new TH1D(name.c_str(), "", bins.nBins, bins.low, bins.high);

    h->GetXaxis()->SetTitle( (bins.xAxis).c_str() );
    h->GetYaxis()->SetTitle( (bins.yAxis).c_str() );

    h->SetLineColor(lColor);
    h->SetMarkerColor(lColor);
    h->SetMarkerStyle(mStyle);
    h->SetLineWidth(lWidth);
    h->SetLineStyle(lStyle);
    h->Sumw2();

    if ( name.find ("ltiplicity") != std::string::npos )
    {
        h->GetXaxis()->SetNdivisions( h->GetNbinsX() );
        h->GetXaxis()->CenterLabels(true);
    }

    return h;
}

TH1F * helpers::myTH1F(std::string name, hBins bins, int lColor, int lWidth, int lStyle, int mStyle )
{
    TH1F* h = new TH1F(name.c_str(), "", bins.nBins, bins.low, bins.high);

    h->GetXaxis()->SetTitle( (bins.xAxis).c_str() );
    h->GetYaxis()->SetTitle( (bins.yAxis).c_str() );

    h->SetLineColor(lColor);
    h->SetMarkerColor(lColor);
    h->SetMarkerStyle(mStyle);
    h->SetLineWidth(lWidth);
    h->SetLineStyle(lStyle);
    h->Sumw2();

    if ( name.find ("ltiplicity") != std::string::npos )
    {
        h->GetXaxis()->SetNdivisions( h->GetNbinsX() );
        h->GetXaxis()->CenterLabels(true);
    }

    return h;
}

TH1D *  helpers::myTH1D(std::string name, hBins bins, const double *xbins, int lColor, int lWidth, int lStyle, int mStyle )
{
    TH1D* h = new TH1D(name.c_str(), "", bins.nBins, xbins );

    h->GetXaxis()->SetTitle( (bins.xAxis).c_str() );
    h->GetYaxis()->SetTitle( (bins.yAxis).c_str() );

    h->SetLineColor(lColor);
    h->SetMarkerColor(lColor);
    h->SetMarkerStyle(mStyle);
    h->SetLineWidth(lWidth);
    h->SetLineStyle(lStyle);
    h->Sumw2();

    if ( name.find ("ultiplicity") != std::string::npos )
    {
        h->GetXaxis()->SetNdivisions( h->GetNbinsX() );
        h->GetXaxis()->CenterLabels(true);
    }

    return h;
}

TH1F * helpers::myTH1F(std::string name, hBins bins, const double *xbins, int lColor, int lWidth, int lStyle, int mStyle )
{
    TH1F* h = new TH1F(name.c_str(), "", bins.nBins, xbins );

    h->GetXaxis()->SetTitle( (bins.xAxis).c_str() );
    h->GetYaxis()->SetTitle( (bins.yAxis).c_str() );

    h->SetLineColor(lColor);
    h->SetMarkerColor(lColor);
    h->SetMarkerStyle(mStyle);
    h->SetLineWidth(lWidth);
    h->SetLineStyle(lStyle);
    h->Sumw2();

    if ( name.find ("ultiplicity") != std::string::npos )
    {
        h->GetXaxis()->SetNdivisions( h->GetNbinsX() );
        h->GetXaxis()->CenterLabels(true);
    }

    return h;
}

TH2D * helpers::myTH2D(std::string name, hBins binsX, hBins binsY )
{
    TH2D* h = new TH2D(name.c_str(), "", binsX.nBins, binsX.low, binsX.high, binsY.nBins, binsY.low, binsY.high);

    h->GetXaxis()->SetTitle( (binsX.xAxis).c_str() );
    h->GetYaxis()->SetTitle( (binsY.yAxis).c_str() );

    return h;  
}

TH2F * helpers::myTH2F(std::string name, hBins binsX, hBins binsY )
{
    TH2F* h = new TH2F(name.c_str(), "", binsX.nBins, binsX.low, binsX.high, binsY.nBins, binsY.low, binsY.high);

    h->GetXaxis()->SetTitle( (binsX.xAxis).c_str() );
    h->GetYaxis()->SetTitle( (binsY.yAxis).c_str() );

    return h;  
}

//_____________________________________________________________________________
std::string helpers::flukaParticle(int particleCode)
{
  std::string toReturn = "unknown_" + itos(particleCode);
    switch ( particleCode )
    {
      case 1:
        toReturn = "Proton";
        break;
      case 2:
        toReturn = "AntiProton";
        break;
      case 3:
        toReturn = "Electron";
        break;
      case 4:
        toReturn = "Positron";
        break;
      case 5:
        toReturn = "Neutrino_Electron";
        break;
      case 6:
        toReturn = "AntiNeutrino_Electron";
        break;
      case 7:
        toReturn = "Photon";
        break;
      case 8:
        toReturn = "Neutron";
        break;
      case 9:
        toReturn = "AntiNeutron";
        break;
      case 10:
        toReturn = "MuonPlus";
        break;
      case 11:
        toReturn = "MuonMinus";
        break;
      case 12:
        toReturn = "KaonLong";
        break;
      case 13:
        toReturn = "PionPlus";
        break;
      case 14:
        toReturn = "PionMinus";
        break;
      case 15:
        toReturn = "KaonPlus";
        break;
      case 16:
        toReturn = "KaonMinus";
        break;
      case 17:
        toReturn = "Lambda";
        break;
      case 18:
        toReturn = "AntiLambda";
        break;
      case 19:
        toReturn = "KaonShort";
        break;
      case 20:
        toReturn = "SigmaMinus";
        break;
      case 21:
        toReturn = "SigmaPlus";
        break;
      case 22:
        toReturn = "SigmaZero";
        break;
      case 23:
        toReturn = "PionZero";
        break;
      case 24:
        toReturn = "KaonZero";
        break;
      case 25:
        toReturn = "AntiKaonZero";
        break;
      case 27:
        toReturn = "Neutrino_Muon";
        break;
      case 28:
        toReturn = "AntiNeutrino_Muon";
        break;
      case 41:
        toReturn = "TauPlus";
        break;
      case 42:
        toReturn = "TauMinus";
        break;
      case 43:
        toReturn = "Neutrino_Tau";
        break;
      case 44:
        toReturn = "AntiNeutrino_Tau";
        break;   
    }
    
    return toReturn;
}


//_____________________________________________________________________________
std::string helpers::pdgParticle(int particleCode)
{
  std::string toReturn = "unknown_" + itos(particleCode);
    switch ( particleCode )
    {
      case 2212:
        toReturn = "Proton";
        break;
      case -2212:
        toReturn = "AntiProton";
        break;
      case 11:
        toReturn = "Electron";
        break;
      case -11:
        toReturn = "Positron";
        break;
      case 12:
        toReturn = "Neutrino_Electron";
        break;
      case -12:
        toReturn = "AntiNeutrino_Electron";
        break;
      case 22:
        toReturn = "Photon";
        break;
      case 2112:
        toReturn = "Neutron";
        break;
      case -2112:
        toReturn = "AntiNeutron";
        break;
      case -13:
        toReturn = "MuonPlus";
        break;
      case 13:
        toReturn = "MuonMinus";
        break;
      case 130:
        toReturn = "KaonLong";
        break;
      case 211:
        toReturn = "PionPlus";
        break;
      case -211:
        toReturn = "PionMinus";
        break;
      case 321:
        toReturn = "KaonPlus";
        break;
      case -321:
        toReturn = "KaonMinus";
        break;
      case 3122:
        toReturn = "Lambda";
        break;
      case -3122:
        toReturn = "AntiLambda";
        break;
      case 310:
        toReturn = "KaonShort";
        break;
      case 3112:
        toReturn = "SigmaMinus";
        break;
      case 3222:
        toReturn = "SigmaPlus";
        break;
      case 3212:
        toReturn = "SigmaZero";
        break;
      case 111:
        toReturn = "PionZero";
        break;
      case 311:
        toReturn = "KaonZero";
        break;
      case -311:
        toReturn = "AntiKaonZero";
        break;
      case 14:
        toReturn = "Neutrino_Muon";
        break;
      case -14:
        toReturn = "AntiNeutrino_Muon";
        break;
      case -15:
        toReturn = "TauPlus";
        break;
      case 15:
        toReturn = "TauMinus";
        break;
      case 16:
        toReturn = "Neutrino_Tau";
        break;
      case -16:
        toReturn = "AntiNeutrino_Tau";
        break;   
    }
    
    return toReturn;
}

int helpers::flukaParticleIdToPythia(int particleCode)
{
  int toReturn = -32;
  
  switch ( particleCode )
  {
    case 1:
      toReturn = 2212;
      break;
    case 2:
      toReturn = -2212;
      break;
    case 3:
      toReturn = 11;
      break;
    case 4:
      toReturn = -11;
      break;
    case 5:
      toReturn = 12;
      break;
    case 6:
      toReturn = -12;
      break;
    case 7:
      toReturn = 22;
      break;
    case 8:
      toReturn = 2112;
      break;
    case 9:
      toReturn = -2112;
      break;
    case 10:
      toReturn = -13;
      break;
    case 11:
      toReturn = 13;
      break;
    case 12:
      toReturn = 130;
      break;
    case 13:
      toReturn = 211;
      break;
    case 14:
      toReturn = -211;
      break;
    case 15:
      toReturn = 321;
      break;
    case 16:
      toReturn = -321;
      break;
    case 17:
      toReturn = 3122;
      break;
    case 18:
      toReturn = -3122;
      break;
    case 19:
      toReturn = 310;
      break;
    case 20:
      toReturn = 3112;
      break;
    case 21:
      toReturn = 3222;
      break;
    case 22:
      toReturn = 3212;
      break;
    case 23:
      toReturn = 111;
      break;
    case 24:
      toReturn = -311;
      break;
    case 25:
      toReturn = -311;
      break;
    case 27:
      toReturn = 14;
      break;
    case 28:
      toReturn = -14;
      break;
    case 41:
      toReturn = -15;
      break;
    case 42:
      toReturn = 15;
      break;
    case 43:
      toReturn = 16;
      break;
    case 44:
      toReturn = -16;
      break;   
  }
  
  return toReturn;
}
