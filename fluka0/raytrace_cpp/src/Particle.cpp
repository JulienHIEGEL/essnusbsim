#include "../include/Particle.h"

ClassImp(Particle)

Particle::Particle()
{
  event = 0;
  id = 0;
  x = y = z = px = py = pz = yOut = xOut = -32;
  
  pT_out = pL_out = -32;
}

Particle::Particle(Double_t ev_, UInt_t i, Double_t x_, Double_t y_, Double_t z_, Double_t px_, Double_t py_, Double_t pz_) :
  event(ev_),id(i),x(x_),y(y_),z(z_),px(px_),py(py_),pz(pz_)
{
  pT_out = pL_out = yOut = xOut = -32;
}

Particle::~Particle()
{
  
}
