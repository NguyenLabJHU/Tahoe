/* base class */
#include "BandT.h"

#include "SSEnhLocDieterichT.h"

#ifndef _DIETERICH_BAND_T_H_
#define _DIETERICH_BAND_T_H_

namespace Tahoe {

  /* forward delaration */
  class SSEnhLocDieterichT;

  class DieterichBandT: public BandT
    { 

    public:

      DieterichBandT(dArrayT normal, dArrayT slipDir, dArrayT
		     perpSlipDir, dArrayT coords,
				   double fH_delta_0, double
				   residCohesion, ArrayT<dSymMatrixT> stressList,
				   SSEnhLocDieterichT* element, double
	       theta_0);

      virtual double Theta();
      virtual double DeltaTheta();
      virtual void StoreDeltaTheta(double deltaTheta);
      //virtual void UpdateDieterichBand();
      virtual double JumpIncrLast();
      virtual void CloseStep();

    private:

      double fTheta;
      double fDeltaTheta;
      double fLastJumpIncrement;

    };

} //end namespace Tahoe

#endif //_DIETERICH_BAND_T_H_
