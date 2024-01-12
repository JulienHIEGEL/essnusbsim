// Reads data from ascii files produced by 4vectors 

#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#include <TCanvas.h>
#include <TDatabasePDG.h>
#include <TGraph.h>
#include <TFile.h>
#include <TLegend.h>
#include <TObject.h>
#include <TTree.h>
#include <TVectorD.h>

#include "../include/ConfigFile.h"
#include "../include/Helpers.h"

#include "../include/FocusingGeometry.h"
#include "../include/Horn.h"
#include "../include/Particle.h"

using std::cout;
using std::endl;
using std::setprecision;
using std::setw;
using std::vector;
using namespace helpers;

// Horn variables
std::vector<double> current;
std::vector<double> baselines;
std::vector<Double_t> horn1Z;
std::vector<Double_t> horn1Y;

std::vector<Double_t> horn2Z;
std::vector<Double_t> horn2Y;

// Trajectory 
double zStep;
double numPoints;
double yLimitMax;

Long64_t limitEvents;
Long64_t totalEvents = 0;
Long64_t decayDiscardedEvents = 0;
Double_t limitAngle;
Double_t energyMin = 0;
Double_t energyMax = 1000;
Double_t decayPipeLength;
Double_t particleCheckPoint = 32000;

bool verbose, testMode, focusParticles, keepOnlyPions, idealFocusing;

void fluka4vectorsToRoot(TString iFile);
void ParticleTransportAndDecay(Particle *p, Double_t decayLength);
Double_t NeutrinoLabAngle(Double_t beta, Double_t cosThetaStarGauss);
Double_t NeutrinoEnergy(Double_t pionEnergy, Double_t neutrinoLabAngle);
void CanvasDrawHorn(TCanvas * canvas);

void RaytraceTest();

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
  
  std::vector<std::string> inputfnames;
  std::string inputfnames_string;
  
  std::string str_curr,str_baselines,str_horn1z,str_horn1y,str_horn2z,str_horn2y;
  config->readInto( inputfnames_string, "InputFiles" );
  config->readInto( limitEvents, "LimitEvents" );
  config->readInto( str_curr, "Currents" );
  config->readInto( str_baselines, "Baselines" );
  config->readInto( str_horn1z, "Horn1Z" );
  config->readInto( str_horn1y, "Horn1Y" );
  config->readInto( str_horn2z, "Horn2Z" );
  config->readInto( str_horn2y, "Horn2Y" );
  config->readInto( numPoints, "TrajectoryPoints" );
  config->readInto( zStep, "ZStep" );
  config->readInto( yLimitMax, "YMaxLimit" );
  config->readInto( limitAngle, "LimitAngle" );
  config->readInto( verbose, "Verbose" );
  config->readInto( testMode, "TestMode" );
  config->readInto( focusParticles, "FocusParticles" );
  config->readInto( idealFocusing, "IdealFocusing" );
  config->readInto( energyMin, "EnergyMin" );
  config->readInto( energyMax, "EnergyMax" );  
  config->readInto( keepOnlyPions,  "KeepOnlyPions");
  config->readInto( decayPipeLength, "DecayPipeLength");
  config->readInto( particleCheckPoint, "ParticleCheckPoint");
  
  helpers::stringParserLong(str_curr,current);
  helpers::stringParserLong(str_baselines,baselines);
  helpers::stringParserLong(str_horn1z,horn1Z);
  helpers::stringParserLong(str_horn1y,horn1Y);
  helpers::stringParserLong(str_horn2z,horn2Z);
  helpers::stringParserLong(str_horn2y,horn2Y);
  
  for ( unsigned int i = 0; i < horn1Y.size(); ++i )
    horn1Y.at(i) = horn1Y.at(i)/100;
  
  for ( unsigned int i = 0; i < horn2Y.size(); ++i )
    horn2Y.at(i) = horn2Y.at(i)/100;  
  
  inputfnames = helpers::stringParser(inputfnames_string);
  
  cout << "Found " << inputfnames.size() << " files in the list.\n";
  
  if ( testMode )
  {
    RaytraceTest();
    
    return 0;
  }
  
  if ( keepOnlyPions )
    cout << " *** Only Pions will be stored in output files!\n";
  
  cout << "Storing particles with energy: [ " << energyMin << ", " << energyMax << " ]\n";
  
  if ( not focusParticles )
    cout << "No focusing geometry\n";
  else
  {
    if ( idealFocusing )
      cout << "Ideal Focusing geometry\n";
    else
      cout << "Normal Focusing geometry\n";
  }
  
  cout << "Focusing area length = " << baselines.at(2) << "\nDecay pipe length = " << decayPipeLength << endl;
  
  if ( particleCheckPoint != 32000 )
    cout << "Particle check point at " << particleCheckPoint << " meters.\n";
  
  for ( std::vector<std::string>::iterator itr = inputfnames.begin(); itr != inputfnames.end(); ++itr)
    fluka4vectorsToRoot(*itr);
  
  delete config; 
  return 0;
}

void fluka4vectorsToRoot(TString iFile)
{
  TDatabasePDG * pdgdb = TDatabasePDG::Instance(); // to get particle masses
  
  /* initialize random seed: */
  srand ( time(NULL) );
  
  /// Opening input file
  ifstream in;
  in.open(iFile, ifstream::in);
  
  /// Opening output file
  TString oFile(iFile);
  oFile.Remove(oFile.Length()-4,4);
  TString app = "_";
  
  if ( not focusParticles )
    app += "NoFocus_";
  else
  {
    if ( idealFocusing )
      app += "IdealFocus_";
    else
      app += "NormalFocus_";
  }
  
  if ( keepOnlyPions )
    app += "Pions_";
  else
    app += "All_";
  
  app += itos(decayPipeLength) + "m";
  
  if ( particleCheckPoint != 32000 )
    app += "_Checkpoint"+itos(particleCheckPoint)+"m";
  
  oFile.Append(app);
  
  cout << "Writing... " << oFile.Append(".root") << endl;
  TFile *f = new TFile(oFile,"RECREATE");
  
  /// Creating tree
  TTree *tree = new TTree("target","4vectors data");
  /// Creating branch
  Particle *p = new Particle;
  tree->Branch("particle",&p, 32000, 0);
  
  /// Creating histograms to be stored in the same rootfile as the tree
  TH1D * h1y = new TH1D("horn1y","",horn1Y.size(), 0, horn1Y.size());
  TH1D * h1z = new TH1D("horn1z","",horn1Z.size(), 0, horn1Z.size());
  TH1D * h2y = new TH1D("horn2y","",horn2Y.size(), 0, horn2Y.size());
  TH1D * h2z = new TH1D("horn2z","",horn2Z.size(), 0, horn2Z.size());
  TH1D * b = new TH1D("baselines","",baselines.size(), 0, baselines.size());
  TH1D * c = new TH1D("currents","",current.size(), 0, current.size());
  
  TH1D * events = new TH1D("events","",3, 0, 3);
  
  /// Filling the histograms
  for ( unsigned int i = 0; i < horn1Y.size(); ++i )
  {
    h1y->SetBinContent(i+1, horn1Y.at(i)*100);
    h1z->SetBinContent(i+1, horn1Z.at(i)*100);
  }
  
  for ( unsigned int i = 0; i < horn2Y.size(); ++i )
  {
    h2y->SetBinContent(i+1, horn2Y.at(i)*100);
    h2z->SetBinContent(i+1, horn2Z.at(i)*100);
  }
  
  for ( unsigned int i = 0; i < baselines.size(); ++i )
    b->SetBinContent(i+1, baselines.at(i));
  
  for ( unsigned int i = 0; i < current.size(); ++i )
    c->SetBinContent(i+1, current.at(i));
  
  
  Int_t nlines = 0;
  
  cout << "Creating horns... 1... ";
  Horn horn1(current.at(0),baselines.at(0),horn1Y.at(horn1Y.size() - 2),baselines.at(0) + horn1Z.at(horn1Z.size() - 3),horn1Y,horn1Z);
  cout << "2... ";
  Horn horn2(current.at(1),baselines.at(1),horn2Y.at(horn2Y.size() - 2),baselines.at(1) + horn2Z.at(horn2Z.size() - 3),horn2Y,horn2Z);
  cout << "OK\n";
  
  cout << "Creating geometry... ";
  FocusingGeometry fg(&horn1, &horn2, baselines);
  cout << "OK\n";
  
  /// Looping over all lines in the text file
  while ( !in.eof() && (limitEvents == -1 || nlines < limitEvents) ) 
  {
    ++totalEvents;
    
    if ( totalEvents%1000 == 0 )
      cout << "Reading entry #"  << totalEvents <<"\r" << std::flush;    
    
    /// Reading and storing information in branch variables
      in >> p->id >> p->x >> p->y >> p->z >> p->px >> p->py >> p->pz;
      
      p->SetEventNum(totalEvents);
      
      if ( keepOnlyPions && !( p->id == PION_P || p->id == PION_M ) )
        continue;
      
      // convert to meters
        p->x = p->x/100; 
        p->y = p->y/100;
        p->z = p->z/100;
        
        p->mass = pdgdb->GetParticle(helpers::flukaParticleIdToPythia(p->id))->Mass();
        
        p->energy = sqrt(p->mass*p->mass + p->px*p->px + p->py*p->py + p->pz*p->pz);
        
        if ( p->energy < energyMin || p->energy > energyMax ) continue;
        
        if ( limitAngle > 0 && (1000*atan( p->Pt()/p->Pl())) > limitAngle ) { continue; }
        
        if ( p->id == PION_M || p->id == PION_P )
        {
          ParticleTransportAndDecay(p, 0.055);
          
          if ( p->GetDecayPos() == 32000 ) 
          {
            ++decayDiscardedEvents;
            continue;
          }
        }
        
        if ( p->id == KAON_M || p->id == KAON_P )
        {
          ParticleTransportAndDecay(p, 0.00747);
          
          if ( p->GetDecayPos() == 32000 ) 
          {
            ++decayDiscardedEvents;
            continue;
          }
        }    
        
        // need to reset it for every particle
        fg.SetCheckpointZ(particleCheckPoint);
        
        if ( not focusParticles )
          fg.NoFocus(p);
        else
        {
          if ( idealFocusing )
            fg.IdealFocus(p);
          else
            fg.PropagateParticle(p);
        }
        
        // to discard events thrown away in focusing
        if ( p->GetDecayPos() == 32000 ) 
        {
          ++decayDiscardedEvents;
          continue;
        }
        
        if ( verbose )
        {
          cout << "mrad in = "  << setprecision(7) << setw(5) << (1000*atan( p->Pt()/p->Pl())) << " out " << p->ThetaOut() << endl;
          cout << "\t In  : pL = " << p->Pl() << " pT = " << p->Pt() << endl;
          cout << "\t        y = " << p->y <<     " z = " << p->z << endl;    
          cout << "\t Out : pL = " << p->Pl_Out() << " pT = " << p->Pt_Out() << endl;  
          cout << "\t        z = " << p->GetZOut() << " y = " << p->GetYOut() << endl;    
        }
        
        /// Filling tree
        tree->Fill();
        
        /// Cross-check that reading the file goes well
        if (!in.good()) break;
        
        ++nlines;
  }
  
  events->SetBinContent( 1, totalEvents);   // total
  events->SetBinContent( 2, nlines );       // kept
  events->SetBinContent( 3, decayDiscardedEvents ); //discarded
  
  cout << "Total events : " << totalEvents << "; kept = " << nlines << "; discarded " << decayDiscardedEvents << " events due to particles decaying after decay pipe length\n";
  
  cout << endl;
  delete p;
  in.close();  /// closing input file
  f->Write();  /// writing output file
  f->Close();  /// closing output file
}


//_____________________________________________________________________________
void ParticleTransportAndDecay(Particle * p, Double_t decayLength )
{
  // assuming max path length equals the path length of a particle with a
  // 45 degree entry and exit angle : -> / \ ->
  // probably a too generous assumption
  Double_t totalL = 2*(baselines.at(2)+decayPipeLength)/1.41;
  Double_t l0 = decayLength * p->P() * 1000;        // energy in MeV
  Double_t totalP = 1 - exp(-(totalL)/l0);    // decay probability
  
  Double_t r = (rand() % 100001)/100000.;     // random number 
  
  if ( r > totalP )  // particle does not decay within total length
  {
    p->SetDecayPos(32000);
    return;
  }
  
  // if the particle decays, we need to define the exact decay position
  Double_t normalization = 1 / (l0*(1-exp(-totalL/l0)));
  
  unsigned int numSteps = 5000;
  Double_t step = totalL / numSteps;
  
  r = (rand() % 100001)/100000.;     // second random number to define position
  
  for ( unsigned int currStep = 1; currStep <= numSteps; ++currStep )
  {
    Double_t currDecayProbability = l0*(1-exp(-(currStep*step)/l0))*normalization;
    
    if ( r <= currDecayProbability )
    {
      p->SetDecayPos(currStep*step);
      
      break;
    }
  }
}



void RaytraceTest()
{
  TDatabasePDG * pdgdb = TDatabasePDG::Instance(); // to get particle masses
  
  // Output file
  cout << "Writing... RaytraceTest.root" << endl;
  TFile *f = new TFile("RaytraceTest.root","RECREATE");
  
  TCanvas *canvas = new TCanvas("c1","Horns!",200,10,1000,500);
  
  TTree *tree = new TTree("target","raytrace data");
  Particle *p = new Particle();
  tree->Branch("particle",&p, 32000, 0);
  
  TH1D * h1y = new TH1D("horn1y","",horn1Y.size(), 0, horn1Y.size());
  TH1D * h1z = new TH1D("horn1z","",horn1Z.size(), 0, horn1Z.size());
  TH1D * h2y = new TH1D("horn2y","",horn2Y.size(), 0, horn2Y.size());
  TH1D * h2z = new TH1D("horn2z","",horn2Z.size(), 0, horn2Z.size());
  TH1D * b = new TH1D("baselines","",baselines.size(), 0, baselines.size());
  TH1D * c = new TH1D("currents","",current.size(), 0, current.size());
  
  for ( unsigned int i = 0; i < horn1Y.size(); ++i )
  {
    h1y->SetBinContent(i+1, horn1Y.at(i)*100);
    h1z->SetBinContent(i+1, horn1Z.at(i)*100);
  }
  
  for ( unsigned int i = 0; i < horn2Y.size(); ++i )
  {
    h2y->SetBinContent(i+1, horn2Y.at(i)*100);
    h2z->SetBinContent(i+1, horn2Z.at(i)*100);
  }
  
  for ( unsigned int i = 0; i < baselines.size(); ++i )
    b->SetBinContent(i+1, baselines.at(i));
  
  for ( unsigned int i = 0; i < current.size(); ++i )
    c->SetBinContent(i+1, current.at(i));
  
  Horn horn1(current.at(0),baselines.at(0),horn1Y.at(horn1Y.size() - 2),baselines.at(0) + horn1Z.at(horn1Z.size() - 3),horn1Y,horn1Z);
  Horn horn2(current.at(1),baselines.at(1),horn2Y.at(horn2Y.size() - 2),baselines.at(1) + horn2Z.at(horn2Z.size() - 3),horn2Y,horn2Z);
  FocusingGeometry fg(&horn1, &horn2, baselines);
  
  if ( limitAngle < 0 )
    limitAngle = 40;
  
  for ( Double_t mrad = 0; mrad < limitAngle; ++mrad)
  {
    canvas->SetName((string("energy: ")+itos(energyMax)+"; "+itos(mrad)+" mrad").c_str());
    CanvasDrawHorn(canvas);
    
    p->SetEventNum(mrad);
    p->id = 0;
    p->x = 0;
    p->y = 0;
    p->z = 0;
    p->px = 0;
    p->py = energyMax * sin(0.001 * mrad);
    p->pz = energyMax * cos(0.001 * mrad);
    p->mass = pdgdb->GetParticle(211)->Mass(); //pi+
    p->SetDecayPos(320000);
    
    // need to reset it for every particle
    fg.SetCheckpointZ(particleCheckPoint);
    
    if ( not focusParticles )
      fg.NoFocus(p);
    else
    {
      if ( idealFocusing )
        fg.IdealFocus(p);
      else
        fg.PropagateParticle(p);
    }
    
    if ( verbose )
    {
      cout << "mrad in = "  << setprecision(7) << setw(5) << (1000*atan( p->Pt()/p->Pl())) << " out " << p->ThetaOut() << endl;
      cout << "\t In  : pL = " << p->Pl() << " pT = " << p->Pt() << endl;
      //       cout << "\t        y = " << p->y <<     " z = " << p->z << endl;    
      cout << "\t Out : pL = " << p->Pl_Out() << " pT = " << p->Pt_Out() << endl;
      //       cout << "\t        z = " << p->GetZOut() << " y = " << p->GetYOut() << endl;    
    }
    
    TVectorD y(fg.GetNumTrajPoints()+1);
    TVectorD z(fg.GetNumTrajPoints()+1);
    
    std::vector<Double_t> tz = fg.GetTrajZ();
    std::vector<Double_t> ty = fg.GetTrajY();
    
    for (unsigned int j = 0; j < tz.size(); ++j)
    {
        //     cout << i << ")\tZ : " <<  p->GetZ(i) << "\tY : " <<  p->GetY(i) << endl;
        y(j) = ty.at(j)*100;
        z(j) = tz.at(j);
    }
    
    TLegend* leg = new TLegend(0.1,0.93,0.39,0.99);
    leg->SetHeader((string("pion energy: ")+itos(energyMax)+"GeV; entry angle "+itos(mrad)+" mrad").c_str());
    leg->SetMargin(0.15);
    leg->SetFillColor(0);
    leg->Draw("same");    
    
    TGraph * t = new TGraph(z, y);
    t->SetName(itos(mrad).c_str());
    t->SetLineWidth(2);
    t->SetLineColor(kRed);
    t->Draw("same");
      
//     t->Write();
    canvas->Write();
    canvas->Print((string("execOut/energy")+itos(energyMax)+"_mrad_"+CorrStr(mrad)+".png").c_str());
    tree->Fill();
  }
  
  f->Write();
}



void CanvasDrawHorn(TCanvas * canvas)
{
  Double_t Horn1Z[horn1Y.size()];
  Double_t Horn1Y[horn1Y.size()];
  Double_t Horn1YM[horn1Y.size()];
  Double_t Horn2Z[horn2Y.size()];
  Double_t Horn2Y[horn2Y.size()];
  Double_t Horn2YM[horn2Y.size()];
  
  std::copy(horn1Z.begin(), horn1Z.end(), Horn1Z);
  std::copy(horn1Y.begin(), horn1Y.end(), Horn1Y);
  std::copy(horn2Z.begin(), horn2Z.end(), Horn2Z);
  std::copy(horn2Y.begin(), horn2Y.end(), Horn2Y);
  
  for (int i = 0; i<horn1Y.size(); ++i)
  {
    Horn1Y[i] = Horn1Y[i]*100;
    Horn1Z[i] += baselines.at(0);
    Horn1YM[i] = -Horn1Y[i];
  }
  
  for (int i = 0; i<horn2Y.size(); ++i)
  {
    Horn2Y[i] = Horn2Y[i]*100;
    Horn2Z[i] += baselines.at(1);
    Horn2YM[i] = -Horn2Y[i];
  }  
    
  TGraph * h1up = new TGraph(horn1Y.size(), Horn1Z, Horn1Y);
  TGraph * h1dw = new TGraph(horn1Y.size(), Horn1Z, Horn1YM);
  TGraph * h2up = new TGraph(horn2Y.size(), Horn2Z, Horn2Y);
  TGraph * h2dw = new TGraph(horn2Y.size(), Horn2Z, Horn2YM);

  TH1F *hr = canvas->DrawFrame(0.001,-80, 100,80);
  h1up->SetLineColor(kBlue);
  h1dw->SetLineColor(kBlue);
  h2up->SetLineColor(kBlue);
  h2dw->SetLineColor(kBlue);

  h1up->SetLineWidth(2);
  h1dw->SetLineWidth(2);
  h2up->SetLineWidth(2);
  h2dw->SetLineWidth(2);

  h1up->Draw("L");
  h1dw->Draw("L same");
  h2up->Draw("L same");
  h2dw->Draw("L same");     
}

