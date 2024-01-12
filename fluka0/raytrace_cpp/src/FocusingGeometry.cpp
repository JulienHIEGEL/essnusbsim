#include "../include/FocusingGeometry.h"

#include <iostream>

using std::cout;
using std::endl;

ClassImp(Particle)

FocusingGeometry::FocusingGeometry()
{
  numPoints = 50000;
  zStep = 0.05;
  yLimitMax = 0.6; 
  VerbosityLevel = 0;
  checkpointZ = 32000;
  checkpointY = 32000;
  
  initialized = false;
}

FocusingGeometry::FocusingGeometry(Horn * h1, Horn * h2, std::vector<double> b)
  : h1_(h1), h2_(h2), baselines(b)
{

  numPoints = 5000;
  zStep = 0.005;
  yLimitMax = 0.6; 
  VerbosityLevel = 0;
  initialized = true;
  checkpointZ = 32000;
  checkpointY = 32000;
  
  if ( VerbosityLevel >= 2 )
  {
    cout << "\nInitialized with:\nHorn 1:\n";
    for ( unsigned int i = 0; i < h1_->Points(); ++i )
      cout << "( " << h1_->Start()+h1_->Z(i) << ", " << h1_->Y(i) << ")\t";
    cout << "\nHorn 2:\n";
    for ( unsigned int i = 0; i < h2_->Points(); ++i )
      cout << "( " << h2_->Start()+h2_->Z(i) << ", " << h2_->Y(i) << ")\t";
    cout << "\n";
  }
}

FocusingGeometry::~FocusingGeometry()
{;}

//_____________________________________________________________________________
void FocusingGeometry::PropagateParticle(Particle * p)
{
  if ( not initialized ) 
  {
    cout << " *** Trying to focus particles with a non-initialized geometry!\n";
    return;
  }
  
  Double_t pTot = sqrt(p->py*p->py+p->pz*p->pz); // incoming particle energy (GeV)

  unsigned int storeEvery = baselines.at(2)/numPoints/zStep;
  
  trajX = std::vector<Double_t>(numPoints+1, 0);
  trajY = std::vector<Double_t>(numPoints+1, 0);
  trajZ = std::vector<Double_t>(numPoints+1, 0);
  
  // if the particle has an X component
//   Double_t turnX = p->py/(sqrt( p->py*p->py ));
// 
//   Double_t bHorn1 = -0.06*h1_->Current()*turnX/pTot;
//   Double_t bHorn2 = -0.06*h2_->Current()*turnX/pTot;
  
  Double_t bHorn1 = -0.06*h1_->Current()/pTot;
  Double_t bHorn2 = -0.06*h2_->Current()/pTot;
  
  Double_t pT = p->py;
  Double_t pL = p->pz;
  
  Double_t yStart = p->y;
  Double_t zStart = p->z;
  
  Double_t yCurr = yStart;
  Double_t zCurr = zStart;
  
  Double_t totalPath = 0;
  
  Double_t angleIn = pT/pL;
  Double_t angleCurr = angleIn;
  
  Double_t alpha = angleIn/sqrt(1.+angleIn*angleIn);
  Double_t delta = alpha;
  unsigned long currPoint = 0;
  checkpointY = 32000;
  
  if ( VerbosityLevel >= 1 ) 
  {
    cout << " >>> Start : " << " Y: " << yCurr << " Z: " << zCurr << " angleIn: " << angleIn << " Baselines : y = " << yLimitMax << " z = " << baselines.at(2)  << endl;   
    cout << " >>> storeEvery : " << storeEvery << " numPoints : " << numPoints << " zStep: " << zStep << endl;
    cout << " >>> decayPos : " << p->GetDecayPos() <<  endl;
  }
  
  // tracing particle trajectory
  for ( unsigned int currStep = 1; currStep <= storeEvery*numPoints; ++currStep )
  {
    yCurr += angleCurr*zStep;
    zCurr = zStep * currStep + zStart;
    
    totalPath += sqrt(zStep * zStep + angleCurr*zStep*angleCurr*zStep); /// %%% TODO %%%
        
    if ( VerbosityLevel == 2 ) 
      cout << " >>> CurrPoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << " totalPath: " << totalPath << endl;
    
    // reached end of focusing volume, particle may be decaying in decay pipe
    if ( zCurr > baselines.at(2) ) break;
    
    // if particle escapes from the y direction, consider it lost
    if ( fabs(yCurr) > yLimitMax )
    {
      if ( VerbosityLevel >= 1 ) 
        cout << " >>> DiscardPoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << " totalPath: " << totalPath << " decayPos: " << p->GetDecayPos() << endl;      
      
      p->SetDecayPos(32000);
      
      break;
    }

    if ( checkpointY == 32000 && zCurr >= checkpointZ )
    {
      checkpointY = yCurr;
      checkpointZ = zCurr;
      p->SetCheckpointZ(zCurr);
      p->SetCheckpointY(yCurr);
      
      if ( VerbosityLevel >= 1 ) 
        cout << " >>> CheckPoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << endl;      
    }
    
    // if we exceed the decay path length particle decays in focusing volume
    // therefore we stop tracking 
    if ( totalPath > p->GetDecayPos() )
    {
      if ( VerbosityLevel >= 1 ) 
        cout << " >>> BreakPoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << " totalPath: " << totalPath << " decayPos: " << p->GetDecayPos() << endl;      
      
      break;
    }

    if ( currStep%storeEvery == 0 )
    {
      currPoint = currStep/storeEvery;
      trajY.at(currPoint) = yCurr;
      trajZ.at(currPoint) = zCurr;
      
      if ( VerbosityLevel >= 1 ) 
        cout << " >>> StorePoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << " totalPath: " << totalPath << endl;
    }
    
    if ( fabs(delta) >= 1 ) break;
    
    angleCurr = delta/sqrt( 1 - delta*delta );
    
    UpdateHorn(h1_, yCurr, zCurr, alpha, delta, bHorn1, angleCurr );
    UpdateHorn(h2_, yCurr, zCurr, alpha, delta, bHorn2, angleCurr );
  }

  // fill remaining positions 
  while ( currPoint < trajZ.size() )
  {
    trajY.at(currPoint) = yCurr;
    trajZ.at(currPoint) = zCurr;
    
    ++currPoint;
  }

  if ( VerbosityLevel >= 1 ) 
    cout << " >>> OutPoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << endl;
  
  // set computed quantities
  p->SetXOut(p->x); 
  p->SetYOut(yCurr);
  p->SetZOut(zCurr);
  
  p->SetPt_Out(pTot * alpha);
  p->SetPl_Out(sqrt(pTot*pTot - p->Pt_Out()*p->Pt_Out()));
}





//_____________________________________________________________________________
void FocusingGeometry::UpdateHorn(Horn * h, Double_t & yCurr, Double_t & zCurr, Double_t & alpha, Double_t & delta, const Double_t bHorn, const Double_t angleCurr )
{
  // what happens when we are in horn
  if ( zCurr > h->Start() && zCurr <= h->End() )
  {
    
    // locate next z coordinate
    unsigned int i = 0;
//     try {

//       cout << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ i = " << i << "\th->Points() = " << h->Points() << endl;
//       cout << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ zCurr = " << zCurr << endl;
//       cout << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ h->Start() + h->Z(i) = " << (h->Start()) << endl;
//       cout << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ h->Z(i) = " << (h->Z(i)) << endl;
      

      
    while ( i < h->Points() - 1 && zCurr > (h->Start() + h->Z(i)) ) 
    {
      i++;

//       cout << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ zCurr = " << zCurr << endl;
//       cout << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ (h->Start() + h->Z(i)) = " << (h->Start() + h->Z(i)) << endl;
      
    }
    
//     for ( i = 1; i < h->Points() - 1; i++)
//     {
// //       cout << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ i = " << i << "\th->Points() = " << h->Points() << endl;
// //       cout << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ zCurr = " << zCurr << endl;
// //       cout << " ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ h->Start() + h->Z(i) = " << (h->Start()+h->Z(i)) << endl;
//      
//       if ( zCurr > (h->Start() + h->Z(i)) ) 
//         break;
//     }
      
//       if ( zCurr > (h->Start() + h->Z(i)) )
//         cout << "zCurr > (h->Start() + h->Z(i)) !!!\n";
     
    // estimate y pos
    Double_t yEstim = h->Y(i-1) + 
              (h->Y(i) - h->Y(i-1)) * (zCurr - h->Start() - h->Z(i-1)) /
                        (h->Z(i) - h->Z(i-1));

    
    if ( fabs(yCurr) > yEstim && fabs(yCurr) <= h->Height() )
    {
      alpha = angleCurr / sqrt(1 + angleCurr*angleCurr);
      delta = alpha + bHorn*zStep/yCurr;
    }
    
//     }
//     catch (std::exception &e)
//     {
//       cout << " ************************************************ Caught exception for i = " << i << "\th->Points() = " << h->Points() << endl;
//       cout << " ************************************************ zCurr = " << zCurr << endl;
//       cout << " ************************************************ (h->Start() + h->Z(i)) = " << (h->Start() + h->Z(i)) << endl;
//       cout << e.what() << endl;
//       throw e;
//       return;
//     }
  }  
}

void FocusingGeometry::IdealFocus(Particle * p)
{
  p->SetPt_Out(0);
  p->SetPl_Out(sqrt(p->px*p->px+p->py*p->py+p->pz*p->pz));
  
  trajX.push_back(p->x);
  trajX.push_back(p->x);
  p->SetXOut(p->x);
  
  trajY.push_back(p->y);
  trajY.push_back(p->y);
  p->SetYOut(p->y);
  
  trajZ.push_back(p->z);
  
  if ( checkpointZ != 32000 )
  {
    checkpointY = p->y;
    p->SetCheckpointY(p->y);
    p->SetCheckpointY(checkpointZ);
  }
  
  if ( p->GetDecayPos() < baselines.at(2) )
  {
    p->SetZOut(p->GetDecayPos());
    trajZ.push_back(p->GetDecayPos());
  }
  else
  {
    p->SetZOut(baselines.at(2));
    trajZ.push_back(baselines.at(2));
  }
  
  if ( VerbosityLevel >= 1 ) 
    cout << " >>> IdealFocus : " << " yOut: " << p->y << "\tzOut: " << p->GetZOut() << "\tPl_Out: " << p->Pl_Out() << endl;       
}



void FocusingGeometry::NoFocus(Particle * p)
{
  yLimitMax = 1; 
  numPoints = 5;
  zStep = 0.1;
  
  p->SetPt_Out(p->py);
  p->SetPl_Out(p->Pl());
  
  Double_t yCurr = p->y;
  Double_t zCurr = p->z;
  Double_t totalPath = 0;
  
  Double_t angleIn = p->py/p->pz;
  
  trajX = std::vector<Double_t>(numPoints+1, 0);
  trajY = std::vector<Double_t>(numPoints+1, 0);
  trajZ = std::vector<Double_t>(numPoints+1, 0);
  
  unsigned int storeEvery = baselines.at(2)/numPoints/zStep;
  unsigned long currPoint = 0;
  
  
  if ( VerbosityLevel >= 1 ) 
  {
    cout << " >>> Start : " << " Y: " << yCurr << " Z: " << zCurr << " angleIn: " << angleIn << " Baselines - y = " << yLimitMax << " z = " << baselines.at(2)  << endl;   
    cout << " >>> storeEvery : " << storeEvery << " numPoints : " << numPoints << " zStep: " << zStep << endl;
  }
  
  for ( unsigned int currStep = 1; currStep <= storeEvery*numPoints; ++currStep )
  {  
    zCurr = zStep * currStep + p->z;
    yCurr = p->y + (zStep * currStep)*angleIn;
    
    totalPath += sqrt(zStep * zStep + angleIn*zStep*angleIn*zStep);
    
    if ( VerbosityLevel >= 2 ) 
      cout << " >>> CurrPoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << endl;
    
    // reached end of focusing volume, particle may be decaying in decay pipe
    if ( zCurr > baselines.at(2) ) break;
    
    // if particle escapes from the y direction it's lost
    if ( fabs(yCurr) > yLimitMax )
    {
      if ( VerbosityLevel >= 1 ) 
        cout << " >>> DiscardPoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << " totalPath: " << totalPath << " decayPos: " << p->GetDecayPos() << endl;      
      
      p->SetDecayPos(32000);
      
      break;
    }
    
    // if we exceed the decay path length particle decays in focusing volume
    // therefore we stop tracking 
    if ( totalPath > p->GetDecayPos() )
    {
      if ( VerbosityLevel >= 1 ) 
        cout << " >>> BreakPoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << " totalPath: " << totalPath << " decayPos: " << p->GetDecayPos() << endl;      
      
      break;
    }

    if ( checkpointY == 32000 && zCurr >= checkpointZ ) 
    {
      checkpointY = yCurr;
      checkpointZ = zCurr;
      p->SetCheckpointZ(zCurr);
      p->SetCheckpointY(yCurr);
      
      if ( VerbosityLevel >= 1 ) 
        cout << " >>> CheckPoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << endl;      
    }
    
    if ( currStep%storeEvery == 0 )
    {
      currPoint = currStep/storeEvery;
      trajZ.at(currPoint) = zCurr;
      trajY.at(currPoint) = yCurr;
      
      if ( VerbosityLevel >= 1 ) 
        cout << " >>> StorePoint : " << currPoint << " Y: " << yCurr << " Z: " << zCurr << endl;      
    }  
  }
  
  // fill remaining positions 
  while ( currPoint < trajZ.size() )
  {
    trajY.at(currPoint) = yCurr;
    trajZ.at(currPoint) = zCurr;
    
    ++currPoint;
  }
  
  p->SetXOut(p->x);
  p->SetYOut(yCurr);
  p->SetZOut(zCurr);
  
}
