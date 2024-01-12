#include "../include/Horn.h"

ClassImp(Horn)

Horn::Horn() : current(0),start(0),end_y(0),end_z(0)
{

}

Horn::Horn(Double_t c,Double_t s,Double_t h,Double_t e,std::vector<Double_t> vy,std::vector<Double_t> vz) :
 current(c),start(s),end_y(h),end_z(e),y(vy),z(vz)
{
  
}

Horn::~Horn()
{

}