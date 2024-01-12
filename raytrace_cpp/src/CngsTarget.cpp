#include <cmath>
#include <iomanip>
#include <iostream>
#include <math.h>

#include <TMath.h>

#include "../include/CngsTarget.h"
#include "../include/ConfigFile.h"
#include "../include/Helpers.h"

using namespace std;
using namespace helpers;

// static const int NUM_ENERGYREGIONS = 2;
// static const int ENERGY_REGIONS[NUM_ENERGYREGIONS][2] = { { 0, 15}, {0, 30} };

static const int NUM_ENERGYREGIONS = 1;
static const int ENERGY_REGIONS[NUM_ENERGYREGIONS][2] = { {0, 30} };

// static const int NUM_ANGLEREGIONS = 8;
// static const int ANGLE_REGIONS[NUM_ANGLEREGIONS][2] = { {3, 6}, { 6, 9 }, { 9, 12 }, { 12, 15 }, { 15, 18 }, { 18, 21 }, { 21, 24 }, { 24, 27 } };

static const int NUM_ANGLEREGIONS = 1;
static const int ANGLE_REGIONS[NUM_ANGLEREGIONS][2] = { { 1, 25 } };

static const double PION_MASS = 0.139570; // GeV
static const double KAON_MASS = 0.493677; // GeV
static const double MUON_MASS = 0.105658; // GeV
static const double ELECTRON_MASS = 0.000511; // GeV

// length from point "0" up to just before the beginning of the decay pipe
static const double HORN_REGION_LENGTH = 100 ; // meters

// how much absorber we have
static const double ABSORBER_LENGTH = 18; // meters

// how much earth we have after the absorber
static const double EARTH_LENGTH = 100; // meters

// average muon energy loss : 1.8 MeV gr/cm^2
static const double MUON_ENERGY_LOSS_ABSORBER = 2; // GeV/meter
static const double MUON_ENERGY_LOSS_EARTH = 0.667; // GeV/meter

// how many neutrinos to generate randomly per pion
static const int DAUGHTERS = 1;
static const float WEIGHT = 1./DAUGHTERS;

const unsigned int NUMFLUXBINS = 6;
const float LIMITS_FLUX_BINS[NUMFLUXBINS][2] = { { 0.5, 2 }, {0.75, 4}, {1, 7}, {2, 7}, {1, 10}, {2, 10} };
const string FLUX_BINS[NUMFLUXBINS] = { "05_2", "075_4", "1_7", "2_7", "1_10", "2_10" };

// max length of the decay pipe
const unsigned int NUMDECAYPIPELENGTH = 7; // convention: first bin always zero
const double DECAY_PIPE_LENGTH[NUMDECAYPIPELENGTH] = { 0, 50, 100, 200, 300, 400, 600 };
static const double DECAY_PIPE_LENGTH_MAX = DECAY_PIPE_LENGTH[NUMDECAYPIPELENGTH-1] ; // meters

// static const int NUM_PARTICLE_TYPES = 16;
// static const int PARTICLE_TYPES[NUM_PARTICLE_TYPES] = { PROTON, APROTON, MUON_P, MUON_M, 
//                                                       KAON_L, PION_P, PION_M,  KAON_P, 
//                                                       KAON_M, LAMBDA, ALAMBDA, KAON_S, 
//                                                       SIGMA_M, SIGMA_P, PI_0, KAON_0};

// static const int NUM_PARTICLE_TYPES = 8;
// static const int PARTICLE_TYPES[NUM_PARTICLE_TYPES] = { KAON_L, PION_P, PION_M,  KAON_P, KAON_M, KAON_S, PI_0, KAON_0};

static const unsigned int NUM_PARTICLE_TYPES = 4;
static const unsigned int PARTICLE_TYPES[NUM_PARTICLE_TYPES] = { PION_P, PION_M, KAON_P, KAON_M };

int main(int argc, char **argv)
{
  if ( argc != 2 )
  {
    cout << "No configuration file specified.\n";
    return 1;
  }
  
  ConfigFile *config;

  try {
    config = new ConfigFile( argv[1] );
  }
  catch ( ConfigFile::file_not_found )
  {
    cout << "Error! Configuration file : \'" << argv[1] << "\' could not be found\n";

    return 1;
  }
  
  /* initialize random seed: */
  srand ( time(NULL) );
  
  CngsTarget *cn;
  
  std::vector<std::string> inputfnames;
  std::string inputfnames_string;
  
  config->readInto( inputfnames_string, "InputFiles" );

  Long64_t lim = -1;
  config->readInto( lim, "LimitEvents" );
  
  inputfnames = helpers::stringParser(inputfnames_string);
  
  for ( std::vector<std::string>::iterator itr = inputfnames.begin(); itr != inputfnames.end(); ++itr)
  {
    cn = new CngsTarget(*itr);
    
    cn->Loop(lim);
    delete cn;
  }
  
  return 0;
}

CngsTarget::CngsTarget(std::string iFile) : fChain(0),inputFile(iFile) 
{
  TTree *tree;

  TFile *f = new TFile(inputFile.c_str());
  f->GetObject("target",tree);
   
  Init(tree);
}


//_____________________________________________________________________________
CngsTarget::~CngsTarget()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}


//_____________________________________________________________________________
void CngsTarget::Loop(Long64_t limitEvents)
{
  time_t start,end;
  time (&start);
  
  //output file
  string plots = "output/Plots_" + inputFile.substr( inputFile.find_last_of("/") + 1, inputFile.find_last_of(".root") - inputFile.find_last_of("/") - 5 );  
  plots += ".root";
  cout << " --- Creating " << plots << " ";
  hFile = new TFile(plots.c_str(),"RECREATE");
  cout << "done\n";
  
  cout << " --- Booking histograms " << "(weight = " << WEIGHT << ")... ";
  BookHistograms();
  cout << "done\n";
  
  if (fChain == 0) return;

  Long64_t nentries = fChain->GetEntries();
  
  if ( nentries > limitEvents + 1 && limitEvents != -1 )
    nentries = limitEvents+1;
  
  // *** Event loop
  for (Long64_t jentry=0; jentry<nentries;jentry++) 
  {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    fChain->GetEntry(jentry);
    
    if ( jentry%10000 == 0 ) 
    {
      time (&end);

      string strLeft;
      double left =  GetTimeLeft((double)jentry/nentries, difftime (end,start));

      if ( left > 75 )
        strLeft = itos(left/60) + " minute(s)";
      else
        strLeft = itos(left) + " seconds";

      std::cout << "[ "  << fixed << setprecision(1) << setw(2) << jentry*100/nentries << "% ] Entry #" << jentry << " ~~~ Time remaining: " << strLeft << "     \r" << std::flush;
    }
       
    particleContent[p->id]++;
    
    // Clearing collections for next pion
    nu_mus.clear();
    nu_es.clear();
    mus.clear();
    es.clear();
    
    // Skip particles not in PARTICLE_TYPES collection
    bool skip = true;
    for ( unsigned int k = 0; k < NUM_PARTICLE_TYPES; ++k )
      if ( p->id == PARTICLE_TYPES[k] )
        skip = false;
    
    if ( skip ) continue;
    
//     if ( p->id == PION_M || p->id == PION_P )
//       for ( int n = 0; n < DAUGHTERS; ++n )
//         GeneratePionDaughters();
//       
//     if ( p->id == KAON_M || p->id == KAON_P )
//       for ( int n = 0; n < DAUGHTERS; ++n )
//         GenerateKaonDaughters();
//       
//     for ( unsigned int m = 0; m < mus.size(); ++m )
//       MuonTransportAndDecay(m, 200);

    FillHistograms();
     
    if ( jentry == limitEvents ) 
    {
      cout << endl << " *** Manual break @" << limitEvents << " events\n";
      break;
    }
  }
  
  std::cout << std::endl;
  
  hFile->Write();
  hFile->Close();
  
  for ( std::map<unsigned int, unsigned long int>::iterator itr = particleContent.begin(); itr != particleContent.end(); ++itr )
  {
    cout << flukaParticle(itr->first) << " : " << itr->second << endl;
  }
}







//_____________________________________________________________________________
void CngsTarget::FillHistograms()
{
  h1_[flukaParticle(p->id)+"_energy"]->Fill(p->energy);
  h1_[flukaParticle(p->id)+"_momentum"]->Fill(sqrt(p->px*p->px+p->py*p->py+p->pz*p->pz));
  h1_[flukaParticle(p->id)+"_pl"]->Fill(fabs(p->pz));
  h1_[flukaParticle(p->id)+"_pt"]->Fill(sqrt(p->px*p->px+p->py*p->py));
  h1_[flukaParticle(p->id)+"_ThetaIn"]->Fill(p->ThetaIn());
    h1_[flukaParticle(p->id)+"_ThetaOut"]->Fill(p->ThetaOut());
  
  if ( p->id == PION_M || p->id == PION_P )
  {
    string whatNu = (p->id == PION_M)? "AntiNeutrinos" : "Neutrinos";
    string whatMu = (p->id == PION_M)? "MuonMinus" : "MuonPlus";
  
    h2_[flukaParticle(p->id)+"_EnergyVsThetaIn"]->Fill(p->energy, fabs(p->ThetaIn()));
    h2_[flukaParticle(p->id)+"_EnergyVsDecayLength"]->Fill( p->energy, p->GetDecayPos() );
    
    for ( unsigned int whichNu = 0; whichNu < nu_mus.size(); ++whichNu )
    {
//       h1_[whatNu + "_ThetaStarCos"]->Fill( nu_mus.at(whichNu).thetaLab );
//       h1_[whatNu + "_ThetaStar"]->Fill( nu_mus.at(whichNu).thetaParent, WEIGHT );
//       h1_[whatNu + "_Theta"]->Fill( acos(nu_mus.at(whichNu).thetaLab*1000), WEIGHT );// lathos!!
      h1_[whatNu + "_ThetaLabMRad"]->Fill( nu_mus.at(whichNu).thetaLab, WEIGHT );
      h1_[whatNu + "_momentum"]->Fill( nu_mus.at(whichNu).momentum, WEIGHT );
//       h1_[whatNu + "_pl"]->Fill( nu_mus.at(whichNu).momentum*nu_mus.at(whichNu).thetaLab, WEIGHT );  
      h1_[whatNu + "_pt"]->Fill( nu_mus.at(whichNu).momentum*sin(nu_mus.at(whichNu).thetaLab), WEIGHT );  
//       h2_[whatNu + "_PionEnergyVs"+whatNu+"Energy"]->Fill( nu_mus.at(whichNu).momentum , p->energy, WEIGHT  );
//       h2_[whatNu + "_PionEnergyVs"+whatNu+"Angle"]->Fill( acos(nu_mus.at(whichNu).thetaLab )*1000 , p->energy, WEIGHT );
      
//       for ( unsigned int f = 0; f < NUMFLUXBINS; ++f )
//       {
//         if ( nu_mus.at(whichNu).momentum >= LIMITS_FLUX_BINS[f][0] && nu_mus.at(whichNu).momentum <= LIMITS_FLUX_BINS[f][1] )
//         {
//           NeutrinoFluxHistograms(FLUX_BINS[f], whichNu);
//         }
//       }
    }
    
//     for ( unsigned int whichMu = 0; whichMu < mus.size(); ++whichMu )
//     {
//       //     h1_[whatMu + "_ThetaStarCos"]->Fill( mus.at(whichMu).thetaParent );
//       //       h1_[whatMu + "_ThetaStar"]->Fill( mus.at(whichMu).thetaParent, WEIGHT );
//       h1_[whatMu + "_Theta"]->Fill( acos(mus.at(whichMu).thetaLab)*1000, WEIGHT );
//       
//       h1_[whatMu + "_momentum"]->Fill( mus.at(whichMu).momentum );
//       //       h1_[whatMu + "_pl"]->Fill( mus.at(whichMu).momentum*mus.at(whichMu).thetaLab, WEIGHT );  
//       h1_[whatMu + "_pt"]->Fill( mus.at(whichMu).momentum*sin(mus.at(whichMu).thetaLab), WEIGHT );  
//             
//       for ( unsigned int f = 0; f < NUMFLUXBINS; ++f )
//       {
//         MuonFluxHistograms(FLUX_BINS[f], whichMu);
//       }      
//     }
  }
  

  for ( int r = 0; r < NUM_ENERGYREGIONS; ++r )
  {
    if ( ( ( p->id == PION_M || p->id == PION_P ) && 
          ( p->energy >= ENERGY_REGIONS[r][0] && p->energy < ENERGY_REGIONS[r][1] ) ) )
    {
      string thisR = itos(ENERGY_REGIONS[r][0]) + "to" + itos(ENERGY_REGIONS[r][1]) + "GeV";
      
//       h1_[flukaParticle(p->id)+"_"+thisR+"_energy"]->Fill(p->energy);
//       h1_[flukaParticle(p->id)+"_"+thisR+"_momentum"]->Fill(sqrt(p->px*p->px+p->py*p->py+p->pz*p->pz));
//       h1_[flukaParticle(p->id)+"_"+thisR+"_pl"]->Fill(fabs(p->pz));
      h1_[flukaParticle(p->id)+"_"+thisR+"_pt"]->Fill(sqrt(p->px*p->px+p->py*p->py));        
      h1_[flukaParticle(p->id)+"_"+thisR+"_ThetaIn"]->Fill(p->ThetaIn());
    }
  }
  
  for ( int r = 0; r < NUM_ANGLEREGIONS; ++r )
  {
    if ( ( p->id == PION_M || p->id == PION_P ) && 
          ( fabs(p->ThetaIn()) >= ANGLE_REGIONS[r][0] && fabs(p->ThetaIn()) < ANGLE_REGIONS[r][1] ) )
    {
      string thisR = itos(ANGLE_REGIONS[r][0]) + "to" + itos(ANGLE_REGIONS[r][1]) + "mrad";
      
      h1_[flukaParticle(p->id)+"_"+thisR+"_energy"]->Fill(p->energy);
//       h1_[flukaParticle(p->id)+"_"+thisR+"_momentum"]->Fill(sqrt(p->px*p->px+p->py*p->py+p->pz*p->pz));
//       h1_[flukaParticle(p->id)+"_"+thisR+"_pl"]->Fill(fabs(p->pz));
      h1_[flukaParticle(p->id)+"_"+thisR+"_pt"]->Fill(sqrt(p->px*p->px+p->py*p->py));        
//       h1_[flukaParticle(p->id)+"_"+thisR+"_ThetaIn"]->Fill(p->ThetaIn());
      
      if ( p->id == PION_M || p->id == PION_P )
        h2_[flukaParticle(p->id)+"_"+thisR+"_EnergyVsDecayLength"]->Fill(p->energy, p->GetDecayPos());
    }
  }

  h_particleContent->Fill(p->id);

}


void CngsTarget::NeutrinoFluxHistograms(string region, unsigned int which)
{
  string whatNu = (p->id == PION_M)? "AntiNeutrinos" : "Neutrinos";
  
  for ( unsigned int dp = 1; dp < NUMDECAYPIPELENGTH; ++dp )
  {
    if ( p->GetDecayPos() <= HORN_REGION_LENGTH + DECAY_PIPE_LENGTH[dp] )
    {
      h1_[whatNu + "_Flux"+region]->Fill( DECAY_PIPE_LENGTH[dp]-1, WEIGHT  );
      h1_[whatNu + "_Flux"+region+"_EnergyPion"+itos(DECAY_PIPE_LENGTH[dp])]->Fill( p->energy );
      h1_[whatNu + "_Flux"+region+"_Mom"+whatNu+itos(DECAY_PIPE_LENGTH[dp])]->Fill( nu_mus.at(which).momentum );
      h1_[whatNu + "_Flux"+region+"_PtPion"+itos(DECAY_PIPE_LENGTH[dp])]->Fill( sqrt(p->px*p->px+p->py*p->py) );
      h1_[whatNu + "_Flux"+region+"_ThetaOutPion"+itos(DECAY_PIPE_LENGTH[dp])]->Fill( p->ThetaOut() );
      
      if ( CrossesNearDetector(DECAY_PIPE_LENGTH[dp] + ABSORBER_LENGTH + EARTH_LENGTH, p->GetDecayPos(), nu_mus.at(which).thetaLab/1000) )
      {
        h1_[whatNu + "_Flux"+region+"_Detector"]->Fill( DECAY_PIPE_LENGTH[dp]-1, WEIGHT );
        h1_[whatNu + "_Flux"+region+"_EnergyPionDetector"+itos(DECAY_PIPE_LENGTH[dp])]->Fill( p->energy );
        h1_[whatNu + "_Flux"+region+"_Mom"+whatNu+"Detector"+itos(DECAY_PIPE_LENGTH[dp])]->Fill( nu_mus.at(which).momentum );
        h1_[whatNu + "_Flux"+region+"_PtPionDetector"+itos(DECAY_PIPE_LENGTH[dp])]->Fill( sqrt(p->px*p->px+p->py*p->py) );
        h1_[whatNu + "_Flux"+region+"_ThetaOutPionDetector"+itos(DECAY_PIPE_LENGTH[dp])]->Fill( p->ThetaOut() );
        h1_[whatNu + "_Flux"+region+"_ThetaLabMRad"+itos(DECAY_PIPE_LENGTH[dp])]->Fill( nu_mus.at(which).thetaLab, WEIGHT );
//         if ( nu_mus.at(which).thetaLab )
//           cout << whatNu << ":" <<  DECAY_PIPE_LENGTH[dp] << ":" << nu_mus.at(which).thetaLab*1000 << endl;
      }
    }
  }
}




//_____________________________________________________________________________
void CngsTarget::MuonFluxHistograms(string region, unsigned int which)
{
  if ( mus.at(which).decayed ) return; // skip decayed muons
  
  string whatMu = (p->id == PION_M)? "MuonMinus" : "MuonPlus";
  
  for ( unsigned int dp = 1; dp < NUMDECAYPIPELENGTH; ++dp )
  {
    if ( p->GetDecayPos() <= HORN_REGION_LENGTH + DECAY_PIPE_LENGTH[dp] )
    {
      h1_[whatMu + "_Flux"+region]->Fill( DECAY_PIPE_LENGTH[dp]-1, WEIGHT  );
      h1_[whatMu + "_Flux"+region+"_Mom"+whatMu+itos(DECAY_PIPE_LENGTH[dp])]->Fill( mus.at(which).momentum );
      
      if ( CrossesNearDetector(DECAY_PIPE_LENGTH[dp] + ABSORBER_LENGTH + EARTH_LENGTH, p->GetDecayPos(), mus.at(which).thetaLab/1000 ) 
            && 
           mus.at(which).momentum > ( ABSORBER_LENGTH*MUON_ENERGY_LOSS_ABSORBER + EARTH_LENGTH*MUON_ENERGY_LOSS_EARTH ) )
      {
        h1_[whatMu + "_Flux"+region+"_Detector"]->Fill( DECAY_PIPE_LENGTH[dp]-1, WEIGHT );
        h1_[whatMu + "_Flux"+region+"_Mom"+whatMu+"Detector"+itos(DECAY_PIPE_LENGTH[dp])]->Fill( mus.at(which).momentum );
      }
    }
  }
}


//_____________________________________________________________________________
bool CngsTarget::CrossesNearDetector(Double_t detectorPos, Double_t decayPos, Double_t thetaLab)
{
  Double_t detectorSide = 5; // (half side) meters

  Double_t yFinal = p->GetYOut() + (detectorPos - decayPos)*sin(thetaLab);
//   Double_t yFinal = p->GetYOut() + (detectorPos - p->GetDecayPos())*sin(thetaLab);
  
  if ( fabs(yFinal) < detectorSide )
    return true;
  else
    return false;
}


//_____________________________________________________________________________
// Generate pion daughters
void CngsTarget::GeneratePionDaughters()
{
  Double_t gamma = p->energy / PION_MASS;
  Double_t beta = sqrt(1 - 1/(gamma*gamma));
  
  long c = 2*M_PI*1000000+1;
  // Neutrino emitted angle
  Double_t nuAngle = (rand() % c)/1000000.; // Pion rest frame
  
  Double_t muAngle = nuAngle < M_PI ? nuAngle + M_PI : nuAngle - M_PI;
  Double_t muLabAngle = LabAngle( beta, muAngle ); 
  Double_t nuLabAngle = LabAngle( beta, nuAngle );
  
  nu_mus.push_back( DecayProduct( p->ThetaOut()+nuLabAngle, nuAngle, NeutrinoEnergy( p->energy, PION_MASS, nuLabAngle ), p->GetDecayPos() ) );
  mus.push_back( DecayProduct( p->ThetaOut()+muLabAngle, muAngle, MuonEnergy( p->energy, PION_MASS, muLabAngle ), p->GetDecayPos() ) );
}




//_____________________________________________________________________________
// Generate kaon daughters
void CngsTarget::GenerateKaonDaughters()
{
  Double_t gamma = p->energy / KAON_MASS;
  Double_t beta = sqrt(1 - 1/(gamma*gamma));  
  
  // decay modes
  double kaonDecayMode = rand() % 10000;
  // does it decay to mu/nu?
  if ( kaonDecayMode <= 6354 ) // 63.54 %
  {
    long c = 2*M_PI*1000000+1;
    // Neutrino emitted angle
    Double_t nuAngle = (rand() % c)/1000000.; // kaon rest frame    
    
    Double_t muAngle = nuAngle < M_PI ? nuAngle + M_PI : nuAngle - M_PI;
    Double_t muLabAngle = LabAngle( beta, muAngle ); 
    Double_t nuLabAngle = LabAngle( beta, nuAngle );
    
    nu_mus.push_back( DecayProduct( p->ThetaOut()+nuLabAngle, nuAngle, NeutrinoEnergy( p->energy, KAON_MASS, nuLabAngle ), p->GetDecayPos() ) );
    mus.push_back( DecayProduct( p->ThetaOut()+muLabAngle, muAngle, MuonEnergy( p->energy, KAON_MASS, muLabAngle ), p->GetDecayPos() ) );
    
  }
  //TODO
//   else if ( kaonDecayMode > 6354 && kaonDecayMode <= 8422 ) // pi+pi0
//   {
//     
//   }
}




void CngsTarget::MuonTransportAndDecay(unsigned int which, Double_t decayPipeLength)
{
  Double_t gamma = sqrt(mus.at(which).momentum*mus.at(which).momentum + MUON_MASS*MUON_MASS) / MUON_MASS;
  Double_t beta = sqrt(1 - 1/(gamma*gamma));  
  
  Double_t t_mu = 2.197e-6; // mean life
  Double_t c = 3e8;
  
  Double_t ld = c*t_mu*gamma*beta; // muon decay length in meters
  
  Double_t distance  = HORN_REGION_LENGTH + decayPipeLength - mus.at(which).genPos;
  double totalP = 1-exp(-distance/ld);   // decay probability in distance
  
  Double_t r = (rand() % 100001)/100000.;     // random number 
  
  if ( r > totalP )  // muon does not decay within length
    return;
  
  // if the particle decays, we need to define the exact decay position
  Double_t normalization = 1 / (ld*(1-exp(-distance/ld)));
  
  unsigned int numSteps = 500;
  Double_t step = distance / numSteps;
  
  r = (rand() % 100001)/100000.;     // second random number to define position
  
  for ( unsigned int currStep = 1; currStep <= numSteps; ++currStep )
  {
    Double_t currDecayProbability = ld*(1-exp(-(currStep*step)/ld))*normalization;
    
    if ( r <= currDecayProbability )
    {
      VegasThreeBodyDecay(which, currStep*step);
      mus.at(which).decayed = true;
     
      break;
    }
  }
}




//_____________________________________________________________________________
// Returns the neutrino angle w.r.t. the pion direction in m-radians
Double_t CngsTarget::LabAngle(Double_t beta, Double_t theta)
{
  if ( theta < M_PI )
    return 1000*(acos((beta + cos(theta))/(1+beta*cos(theta))));
  else
    return 1000*(-acos((beta + cos(theta))/(1+beta*cos(theta))));  
}


//_____________________________________________________________________________
// Returns emitted neutrino energy 
Double_t CngsTarget::NeutrinoEnergy(Double_t particleEnergy, Double_t particleMass, Double_t neutrinoLabAngle)
{
  Double_t gamma = particleEnergy / particleMass;
  Double_t beta = sqrt(1 - 1/(gamma*gamma));
  
  Double_t eNuMax = (particleMass*particleMass - MUON_MASS*MUON_MASS)/(2*particleMass);
  
  return eNuMax / (gamma*(1-beta*cos(neutrinoLabAngle)));
}
    

    
//_____________________________________________________________________________
// Returns emitted muon energy 
Double_t CngsTarget::MuonEnergy(Double_t particleEnergy, Double_t particleMass, Double_t muonLabAngle)
{
  Double_t gamma = particleEnergy / particleMass;
  Double_t beta = sqrt(1 - 1/(gamma*gamma));
  
  Double_t eMuMax = (particleMass*particleMass + MUON_MASS*MUON_MASS)/(2*particleMass);

  return eMuMax / (gamma*(1-beta*cos(muonLabAngle)));
}
    

//_____________________________________________________________________________
void CngsTarget::BookHistograms()
{
  h_particleContent = new TH1D("particleContent","",40 ,0 , 40 );
  
  string y_axis = "# events";                                     
  hBins bin_E       = { y_axis, "E (GeV)"          ,  40000,   0.  , 400.   };
  hBins bin_P       = { y_axis, "P (GeV)"          ,  40000,   0.  , 400.   };
  hBins bin_Pl      = { y_axis, "P_{long} (GeV)"   ,  40000,   0.  , 400.   };
  hBins bin_Pt      = { y_axis, "P_{tran} (GeV)"   ,   6000,   0.  ,   6.   };
  hBins bin_mrad    = { y_axis, "mrad"             ,   2000,  -4000.  , 4000.   };
  hBins bin_mmrad   = { y_axis, "mrad"             ,   6000,  -300.  , 300.   };
  hBins bin_rad     = { y_axis, "rad"              ,    200,  -7.  ,   7.   };
  hBins bin_E_small = { y_axis, "E (GeV)"          ,    200,   0.  ,  50.   };
  hBins bin_dLen    = { "Decay length (m)", ""     ,    DECAY_PIPE_LENGTH_MAX/(NUMDECAYPIPELENGTH-1),  0.  ,DECAY_PIPE_LENGTH[NUMDECAYPIPELENGTH-1]   };
  hBins bin_angle   = { y_axis, "angle"            ,    100,  -4.  ,   4.   };
  
  hBins bin_dLenVar = { "Decayed pions", "Decay length (m)"     ,    NUMDECAYPIPELENGTH-1,  0.  , DECAY_PIPE_LENGTH[NUMDECAYPIPELENGTH-1] };
  
  for ( unsigned int p = 0; p < NUM_PARTICLE_TYPES; ++p )
  {
    string thisP = flukaParticle(PARTICLE_TYPES[p]); // this particle
    MkCdDir( "/", thisP );
    
    h1_[thisP+"_energy"]    = myTH1F(thisP+"_energy",   bin_E);
    h1_[thisP+"_momentum"]  = myTH1F(thisP+"_momentum", bin_P);
    h1_[thisP+"_pl"]        = myTH1F(thisP+"_pl",       bin_Pl);   
    h1_[thisP+"_pt"]        = myTH1F(thisP+"_pt",       bin_Pt);   
    h1_[thisP+"_ThetaIn"]   = myTH1F(thisP+"_ThetaIn",  bin_mrad);  
    h1_[thisP+"_ThetaOut"]  = myTH1F(thisP+"_ThetaOut",  bin_mrad);    
    
    if (  PARTICLE_TYPES[p] == PION_M || PARTICLE_TYPES[p] == PION_P )
    {
      h2_[thisP+"_EnergyVsThetaIn"] = myTH2F( thisP+"_EnergyVsThetaIn", bin_E_small, bin_mrad);
      h2_[thisP+"_EnergyVsDecayLength"] = myTH2F( thisP+"_EnergyVsDecayLength", bin_E_small, bin_dLen);
    }
    
    if ( PARTICLE_TYPES[p] == PION_M || PARTICLE_TYPES[p] == PION_P )
    {
      for ( int r = 0; r < NUM_ENERGYREGIONS; ++r )
      {       
        string thisR = itos(ENERGY_REGIONS[r][0]) + "to" + itos(ENERGY_REGIONS[r][1]) + "GeV";
        MkCdDir( "/"+thisP, thisR );
        
//         h1_[thisP+"_"+thisR+"_energy"]    = myTH1F(thisP+"_"+thisR+"_energy",   bin_E);
//         h1_[thisP+"_"+thisR+"_momentum"]  = myTH1F(thisP+"_"+thisR+"_momentum", bin_P);
//         h1_[thisP+"_"+thisR+"_pl"]        = myTH1F(thisP+"_"+thisR+"_pl",       bin_Pl);         
        h1_[thisP+"_"+thisR+"_pt"]        = myTH1F(thisP+"_"+thisR+"_pt",       bin_Pt);         
        h1_[thisP+"_"+thisR+"_ThetaIn"]   = myTH1F(thisP+"_"+thisR+"_ThetaIn",  bin_mrad);   
      }
      
      for ( int r = 0; r < NUM_ANGLEREGIONS; ++r )
      {
        string thisR = itos(ANGLE_REGIONS[r][0]) + "to" + itos(ANGLE_REGIONS[r][1]) + "mrad";
        MkCdDir( "/"+thisP, thisR );
        
        h1_[thisP+"_"+thisR+"_energy"]    = myTH1F(thisP+"_"+thisR+"_energy",   bin_E);
//         h1_[thisP+"_"+thisR+"_momentum"]  = myTH1F(thisP+"_"+thisR+"_momentum", bin_P);
//         h1_[thisP+"_"+thisR+"_pl"]        = myTH1F(thisP+"_"+thisR+"_pl",       bin_Pl);         
        h1_[thisP+"_"+thisR+"_pt"]        = myTH1F(thisP+"_"+thisR+"_pt",       bin_Pt);         
//         h1_[thisP+"_"+thisR+"_ThetaIn"]   = myTH1F(thisP+"_"+thisR+"_ThetaIn",  bin_mrad);   

        if (  PARTICLE_TYPES[p] == PION_M || PARTICLE_TYPES[p] == PION_P )
        {
          h2_[thisP+"_"+thisR+"_EnergyVsDecayLength"] = myTH2F( thisP+"_"+thisR+"_EnergyVsDecayLength", bin_E_small, bin_dLen);
        }
      }
    }
  }
  
  string daughters[4] = { "Neutrinos", "AntiNeutrinos", "MuonPlus", "MuonMinus" };
  
  for ( unsigned int d = 0; d < 4; ++d )
  {   
    string thisP = daughters[d];
    MkCdDir( "/", thisP );
    
    h1_[thisP+"_momentum"]     = myTH1F(thisP+"_momentum", bin_P);
//     h1_[thisP+"_pl"]           = myTH1F(thisP+"_pl",       bin_Pl);   
    h1_[thisP+"_pt"]           = myTH1F(thisP+"_pt",       bin_Pt);   
//     h1_[thisP+"_ThetaStarCos"] = myTH1F(thisP+"_ThetaStarCos",  bin_angle);  
//     h1_[thisP+"_ThetaStar"]    = myTH1F(thisP+"_ThetaStar",  bin_rad);  
    h1_[thisP+"_Theta"]        = myTH1F(thisP+"_Theta",  bin_mrad);
    h1_[thisP+"_ThetaLabMRad"]        = myTH1F(thisP+"_ThetaLabMRad",  bin_mrad);  
//     h2_[thisP+"_PionEnergyVs"+thisP+"Energy"] = 
//        myTH2F( thisP+"_PionEnergyVs"+thisP+"Energy", bin_E_small, bin_E_small);
//     h2_[thisP+"_PionEnergyVs"+thisP+"Angle"]  = 
//        myTH2F( thisP+"_PionEnergyVs"+thisP+"Angle",  bin_E_small, bin_mrad);
    
     
    for ( unsigned int f = 0; f < NUMFLUXBINS; ++f )
    {
      string thisF = "Flux" + FLUX_BINS[f];
      
      MkCdDir( "/"+thisP, thisF );
      h1_[thisP+"_"+thisF]       = myTH1D(thisP+"_"+thisF, bin_dLenVar, DECAY_PIPE_LENGTH );
      h1_[thisP+"_"+thisF+"_Detector"]     = myTH1D(thisP+"_"+thisF+"_Detector",    bin_dLenVar, DECAY_PIPE_LENGTH );
      
      for ( unsigned int dp = 1; dp < NUMDECAYPIPELENGTH; ++dp )
      {
        h1_[thisP+"_"+thisF+"_Mom"+thisP+itos(DECAY_PIPE_LENGTH[dp])] = 
            myTH1F(thisP+"_"+thisF+"_Mom"+thisP+itos(DECAY_PIPE_LENGTH[dp]),      bin_P);
        h1_[thisP+"_"+thisF+"_Mom"+thisP+"Detector"+itos(DECAY_PIPE_LENGTH[dp])] = 
            myTH1F(thisP+"_"+thisF+"_Mom"+thisP+"Detector"+itos(DECAY_PIPE_LENGTH[dp]),      bin_P);
        
        if ( d < 2 ) // for (anti)neutrinos only
        {
          h1_[thisP+"_"+thisF+"_EnergyPion"+itos(DECAY_PIPE_LENGTH[dp])] = 
              myTH1F(thisP+"_"+thisF+"_EnergyPion"+itos(DECAY_PIPE_LENGTH[dp]),  bin_E);
          h1_[thisP+"_"+thisF+"_EnergyPionDetector"+itos(DECAY_PIPE_LENGTH[dp])] = 
              myTH1F(thisP+"_"+thisF+"_EnergyPionDetector"+itos(DECAY_PIPE_LENGTH[dp]),  bin_E);
          h1_[thisP+"_"+thisF+"_PtPion"+itos(DECAY_PIPE_LENGTH[dp])] = 
              myTH1F(thisP+"_"+thisF+"_PtPion"+itos(DECAY_PIPE_LENGTH[dp]),  bin_Pt);
          h1_[thisP+"_"+thisF+"_PtPionDetector"+itos(DECAY_PIPE_LENGTH[dp])] = 
              myTH1F(thisP+"_"+thisF+"_PtPionDetector"+itos(DECAY_PIPE_LENGTH[dp]),  bin_Pt);
          h1_[thisP+"_"+thisF+"_ThetaOutPion"+itos(DECAY_PIPE_LENGTH[dp])]         = 
              myTH1F(thisP+"_"+thisF+"_ThetaOutPion"+itos(DECAY_PIPE_LENGTH[dp]),  bin_mrad);
          h1_[thisP+"_"+thisF+"_ThetaOutPionDetector"+itos(DECAY_PIPE_LENGTH[dp])] = 
              myTH1F(thisP+"_"+thisF+"_ThetaOutPionDetector"+itos(DECAY_PIPE_LENGTH[dp]),  bin_mrad);
        
          h1_[thisP+"_"+thisF+"_ThetaLabMRad"+itos(DECAY_PIPE_LENGTH[dp])]     = 
              myTH1F(thisP+"_"+thisF+"_ThetaLabMRad"+itos(DECAY_PIPE_LENGTH[dp]),  bin_mmrad);  
        }
      }
    } 
  }
}






//_____________________________________________________________________________
Int_t CngsTarget::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t CngsTarget::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

//_____________________________________________________________________________
void CngsTarget::Init(TTree *tree)
{
   // Set object pointer
   p = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("particle", &p );

   Notify();
}

//_____________________________________________________________________________
Bool_t CngsTarget::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

//_____________________________________________________________________________
void CngsTarget::MkCdDir(std::string root, std::string newdir)
{
    string newDirLocation = root + "/" + newdir;

    hFile->GetDirectory( root.c_str() )->mkdir( newdir.c_str() );
    hFile->cd( newDirLocation.c_str() );
}

//_____________________________________________________________________________
long int CngsTarget::GetTimeLeft(double PercentDone, double SecondsTaken)
{
   //Prevent division by 0
   if(PercentDone == 0.0) return 0;

   //Calculate the time remaining
   return (long int)((SecondsTaken / PercentDone) - SecondsTaken + 0.99999);
}


//_____________________________________________________________________________
// Three-body decay random numbers
// Code in this section has been adapted from 
// BRIDGE - Branching Ratio Inquiry / Decay Generated Events
//                            by Patrick Meade and Matt Reece
// http://www.lepp.cornell.edu/Research/TPP/BridgeSoftware.html
// some comments have been left intact
void CngsTarget::VegasThreeBodyDecay(unsigned int which, Double_t pos)
{
  Interval x1, x2, x3, x4, x5;
  x1.xLow = x2.xLow = x3.xLow = x4.xLow = x5.xLow = 0.;
  x1.xHigh = x2.xHigh = x3.xHigh = x4.xHigh = x5.xHigh = 1.;
  
  Region region;
  region.push_back(x1);
  region.push_back(x2); 
  region.push_back(x3);
  region.push_back(x4);
  region.push_back(x5);
  
  int nDiv = 1;        // number of subdivisions on an axis.
  int nDim = region.size();
  // Initialize the arrays: it's important that there be a "1"
  // in the 1 position, and "0"s above it. We ignore the "0"
  // position (unfortunate remnant of my laziness in Fortran tranlation)
  for(int j = 0; j < nDim; j++) 
  {
    region[j].grid.push_back(1.);
    region[j].grid.push_back(1.);
    
    for(int i = 0; i < 100; i++) 
      region[j].grid.push_back(0.);
  }
  

  // heavily modified/slimmed down vegas integration routine in order to get 
  // only   // one set of numbers -- there's definitely a smarter way to do this
  // so let's call it a TODO  
  int nCall = 50000; // Approx. number of integrand evaluations per iteration.
 
  double xnd;      // saved for getRandomFromGrid... precise definition?
  const int maxNDiv = 50; // maximum number of subdivisions on an axis.
  int nd, ng;
  double xn, xo, dxg, rc;

  std::vector<int> ia(nDim), kg(nDim);
  std::vector<double> r(maxNDiv+1), x(nDim);
  
  nd = maxNDiv;

  ng = int(pow((nCall/2. + 0.25),(1./nDim)));
  dxg=1./ng;
  xnd=nd;
  dxg=dxg*xnd;
  
  //-- do the binning if necessary
  if(nd != nDiv) 
  {
    for(int i = 1; i <= nd; i++) 
      r[i] = 1.;
    
    for (int j = 0; j < nDim; j++) // Rebin axis #j.
      region[j].Rebin(nDiv/xnd, nd, r);
    
    nDiv=nd;
  }

  for(int j = 0; j < nDim; j++) 
    kg[j] = 1;
    
  for(int j = 0; j < nDim; j++) 
  {
    Double_t random = (rand() % 100001) / 100000. ; 
    
    if ( random == 0 || random == 1 )
      random = (rand() % 100001) / 100000. ;
    
    xn = (kg[j] - random) * dxg + 1;
    ia[j] = max(min(int(xn),maxNDiv),1);
    
    if(ia[j] > 1) 
    {
      xo = region[j].grid[ia[j]] - region[j].grid[ia[j]-1];
      rc = region[j].grid[ia[j]-1] + (xn - ia[j])*xo;
    } 
    else 
    {
      xo = region[j].grid[ia[j]];
      rc = (xn - ia[j])*xo;
    }
    
    x[j] = region[j].xLow + rc * region[j].Length();
  }
      
      
  ThreeBodyDecay(x, which, pos);
}

//_____________________________________________________________________________
// Three-body decay 
void CngsTarget::ThreeBodyDecay(vector<double> x, unsigned int which, Double_t pos)
{
  FourVector P[4];  
  
  // setting four-momentum values from decaying muon
  // FIXME: transverse momentum split equally between x,y for now
  P[0].P[1] = mus.at(which).momentum*sin(mus.at(which).thetaLab/1000)/1.4142;
  P[0].P[2] = mus.at(which).momentum*sin(mus.at(which).thetaLab/1000)/1.4142;
  P[0].P[3] = mus.at(which).momentum*cos(mus.at(which).thetaLab/1000);
    
  P[0].E()=sqrt(MUON_MASS*MUON_MASS + P[0].P[1]*P[0].P[1] + P[0].P[2]*P[0].P[2] + P[0].P[3]*P[0].P[3]);
  
  // Save the random numbers, which the rest of the code will use....
  double randIn[5];
  for (unsigned int i = 0; i < x.size(); i++) 
  {
    if (x[i] < 0. || x[i] > 1. || x[i] != x[i] ) 
    {
      // throw an error
      cout << "Invalid x[" << i << "] = " << x[i] << "!\n";
      exit(1);
      return;
    }
    
    randIn[i] = x[i];
  }
  
  double rootS = fabs(P[0].M());  
  
  // Get masses
  double m1 = ELECTRON_MASS; // electron
  double m2 = 0;             // nu e
  double m3 = 0;             // nu mu
  
  double E1max = (rootS*rootS + m1*m1 - m2*m2 - 2*m2*m3 - m3*m3)/(2*rootS);

  double E1 = m1 + randIn[0]*(E1max - m1); 
  double E1jac = E1max - m1;
  
  if(E1jac < 0) 
    cout << "Weird sign: (E1max, m1) = " << E1max << ", " << m1 << "\n";

  double E2min = ((E1-rootS)*(m3*m3-m2*m2-m1*m1+2*E1*rootS-rootS*rootS) - 
  sqrt((E1-m1)*(E1+m1)*((m3-m2)*(m3-m2)-m1*m1+2*E1*rootS-rootS*rootS)*
  ((m3+m2)*(m3+m2)-m1*m1+2*E1*rootS-rootS*rootS)))
  / (2*(m1*m1 + rootS*(-2*E1 + rootS)));
  double E3min = ((E1-rootS)*(m2*m2-m3*m3-m1*m1+2*E1*rootS-rootS*rootS) - 
  sqrt((E1-m1)*(E1+m1)*((m2-m3)*(m2-m3)-m1*m1+2*E1*rootS-rootS*rootS)*
  ((m3+m2)*(m3+m2)-m1*m1+2*E1*rootS-rootS*rootS)))
  / (2*(m1*m1 + rootS*(-2*E1 + rootS)));
  double E2max = rootS - m1 - m3;

  
  if( E3min > rootS - E1 - E2max ) 
    E2max = rootS - E1 - E3min;

  double E2 = E2min + randIn[1]*(E2max-E2min);
  double E2jac = E2max - E2min;
  
  if(E2jac < 0) 
    cout << "Weird sign: (E2max, E2min) = " << E2max << ", " << E2min << "\n";

  double E3 = rootS - E1 - E2;
  double cosTheta = -1. + 2.*randIn[2];
  double sinTheta = sqrt(1 - cosTheta*cosTheta);
  double phi = 2.*M_PI*randIn[3];
  double psi = 2.*M_PI*randIn[4];

  double P1 = sqrt(E1 * E1 - m1 * m1);
  double P2 = sqrt(E2 * E2 - m2 * m2);
  double P3 = sqrt(E3 * E3 - m3 * m3);

  P[1].P[0] = E1;
  double stcp = sinTheta * cos(phi);
  double stsp = sinTheta * sin(phi);
  double ct = cosTheta;
  P[1].P[1] = stcp * P1;
  P[1].P[2] = stsp * P1;
  P[1].P[3] = ct * P1;
  double vtemp[4], vrot[4];
  vtemp[0] = 1;
  vtemp[1] = cos(psi);
  vtemp[2] = sin(psi);
  vtemp[3] = 0;
  rotxxx_(vtemp, P[1].P, vrot);
  double B = (P3*P3-P2*P2-P1*P1)/(2*P1);
  double A;
  
  if ( P2 * P2 - B * B > 0.0 ) 
    A = sqrt(P2 * P2 - B * B);
  else 
    A = 0.0;

  P[2].P[0] = E2;
  P[2].P[1] = A*vrot[1] + B*stcp;
  P[2].P[2] = A*vrot[2] + B*stsp;
  P[2].P[3] = A*vrot[3] + B*ct;
  P[3].P[0] = E3;
  double C = - B - P1;
  P[3].P[1] = -A*vrot[1] + C*stcp;
  P[3].P[2] = -A*vrot[2] + C*stsp;
  P[3].P[3] = -A*vrot[3] + C*ct;
  
  Double_t eAngleNoBoost    = atan2( sqrt(P[1].P[1]*P[1].P[1] + P[1].P[2]*P[1].P[2]), P[1].P[3]);
  Double_t nueAngleNoBoost  = atan2( sqrt(P[2].P[1]*P[2].P[1] + P[2].P[2]*P[2].P[2]), P[2].P[3]);
  Double_t numuAngleNoBoost = atan2( sqrt(P[3].P[1]*P[3].P[1] + P[3].P[2]*P[3].P[2]), P[3].P[3]);

  // boost to lab frame
  BoostFourMom(P[1],P[0],P[1]);
  BoostFourMom(P[2],P[0],P[2]);
  BoostFourMom(P[3],P[0],P[3]);
  
  Double_t eAngleBoost    = atan2( sqrt(P[1].P[1]*P[1].P[1] + P[1].P[2]*P[1].P[2]), P[1].P[3])*1000;
  Double_t nueAngleBoost  = atan2( sqrt(P[2].P[1]*P[2].P[1] + P[2].P[2]*P[2].P[2]), P[2].P[3])*1000;
  Double_t numuAngleBoost = atan2( sqrt(P[3].P[1]*P[3].P[1] + P[3].P[2]*P[3].P[2]), P[3].P[3])*1000;

  es.push_back    ( DecayProduct( eAngleBoost,    eAngleNoBoost,    P[1].P[1]*P[1].P[1]+P[1].P[2]*P[1].P[2]+P[1].P[3]*P[1].P[3], pos ) );
  nu_es.push_back ( DecayProduct( nueAngleBoost,  nueAngleNoBoost,  P[2].P[1]*P[2].P[1]+P[2].P[2]*P[2].P[2]+P[2].P[3]*P[2].P[3], pos ) );
  nu_mus.push_back( DecayProduct( numuAngleBoost, numuAngleNoBoost, P[3].P[1]*P[3].P[1]+P[3].P[2]*P[3].P[2]+P[3].P[3]*P[3].P[3], pos ) );
}


//_____________________________________________________________________________
// Rebin the interval.
// r[i]: how to rebin subinterval #i.
// The idea is to divide up subinterval #i into r[i] subsubintervals,
// but then regroup the new subintervals, the number of subsubintervals
// in each new group being equal to preserve the increment density.
// The optimal grid is obtained when the r[i]'s are equal for all i.
void Interval::Rebin(double rc, int ndiv, std::vector<double> r)
{
  double xo = 0;
  double xn = 0;
  double dr = 0;
  int k = 0;
  xn = 0.;
  dr = 0.;
  vector<double> gridtemp;
  
  for (int i = 1; i < ndiv; i++) 
  {
    while(rc > dr) 
    {
      k++;
      dr += r[k];
      xo = xn;
      xn = grid[k];
    }
    
    dr -= rc;
    
    gridtemp.push_back(xn - (xn-xo)*dr/r[k]);
  }
  
  for(int i = 1; i < ndiv; i++) 
    grid[i] = gridtemp[i-1];
  
  grid[ndiv] = 1.;
  return;
}

// adapted from HELAS library
void CngsTarget::BoostFourMom(FourVector p, FourVector q, FourVector & pboost)
{
  Double_t qq = q.P[1]*q.P[1]+q.P[2]*q.P[2]+q.P[3]*q.P[3];
    
  if ( qq != 0 )
  {
    Double_t pq = p.P[1]*q.P[1]+p.P[2]*q.P[2]+p.P[3]*q.P[3];
    Double_t m = sqrt(q.P[0]*q.P[0]-qq);
    Double_t lf = ((q.P[0]-m)*pq/qq+p.P[0])/m;

    pboost.P[0] = (p.P[0]*q.P[0]+pq)/m;
    pboost.P[1] =  p.P[1]+q.P[1]*lf;
    pboost.P[2] =  p.P[2]+q.P[2]*lf;
    pboost.P[3] =  p.P[3]+q.P[3]*lf;
  }
  else
  {
    pboost.P[0] = p.P[0];
    pboost.P[1] = p.P[1];
    pboost.P[2] = p.P[2];
    pboost.P[3] = p.P[3];
  }
}
