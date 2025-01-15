#include "sndHit.h"


// -----   Default constructor   -------------------------------------------
sndHit::sndHit()
  : TObject(),
    fDetectorID(-1)
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
sndHit::sndHit(Int_t detID,Int_t nP,Int_t nS)
  :TObject(),
   fDetectorID(detID),
   nSiPMs(nP),
   nSides(nS)
  {
     for (unsigned int j=0; j<16; ++j){
        signals[j]  = -1;
        times[j]    =-1;
        fDaqID[j]  =-1;
     }
  }

Float_t sndHit::GetSignal(Int_t nChannel)
{
 return signals[nChannel];
}
Float_t sndHit::GetTime(Int_t nChannel)
{
 return times[nChannel];
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
sndHit::~sndHit() { }
// -------------------------------------------------------------------------


ClassImp(sndHit)
