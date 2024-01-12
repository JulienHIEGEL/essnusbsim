#ifndef horn_h
#define horn_h

#include <vector>

#include "TObject.h"

class Horn : public TObject {
  public:
    Horn();
    Horn(Double_t c,Double_t s,Double_t h,Double_t e,std::vector<Double_t> vy,std::vector<Double_t> vz);
    ~Horn();
    
    void SetStart(Double_t d) { start = d;};
    void SetEnd(Double_t d) { end_z = d;};
    void SetHeight(Double_t d) { end_y = d;};
    
    void SetZ(std::vector<Double_t> v) { z = v; };
    void SetY(std::vector<Double_t> v) { y = v; };
    
    Double_t Start()  { return start; };
    Double_t End()    { return end_z; };
    Double_t Height() { return end_y; };
    
    Double_t Current()    { return current;   };
    
    Double_t Z( size_t n) { return z.at(n); };
    Double_t Y( size_t n) { return y.at(n); };
    
    size_t Points() { return z.size(); };
  
  
  private:
    Double_t current;
    Double_t start;
    Double_t end_y;
    Double_t end_z;
    std::vector<Double_t> y;  
    std::vector<Double_t> z;
    
    ClassDef(Horn,1)
};


#endif
