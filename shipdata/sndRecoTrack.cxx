#include "sndRecoTrack.h"
#include "ShipUnit.h"
#include "TROOT.h"
#include "TMatrixD.h"
#include "TMath.h"
#include "TGraph.h"
#include "TF1.h"
#include "StateOnPlane.h"  /* genfit */
#include "FitStatus.h"
#include "RKTrackRep.h"
#include "ConstField.h"
#include "FieldManager.h"
#include "TGeoMaterialInterface.h"
#include "MaterialEffects.h"

using namespace genfit;
using namespace std;

//tests
#include <iostream>

/* Constructor from genfit::Track object */
sndRecoTrack::sndRecoTrack(Track* track)
{
   FitStatus* fitStatus = track->getFitStatus();
   chi2  = fitStatus->getChi2();
   Ndf   = fitStatus->getNdf();
   fFlag = fitStatus->isFitConverged();
      
   if (fFlag)
   {
      for ( auto i = 0; i < track->getNumPoints(); i++ )
      {
          auto state = track->getFittedState(i);
          fTrackPoints.push_back(state.getPos());
          fRawMeasDetID.push_back(track->getPointWithMeasurement(i)->getRawMeasurement()->getDetId());
          if (i ==0)
          {
             fTrackMom = state.getMom();
             start = state.getPos();
          }
          if (i == track->getNumPoints()-1)
             stop = state.getPos();
      }
   }
   // defaults
   fTrackType = 0;
   fRawMeasTimes = {};
}
