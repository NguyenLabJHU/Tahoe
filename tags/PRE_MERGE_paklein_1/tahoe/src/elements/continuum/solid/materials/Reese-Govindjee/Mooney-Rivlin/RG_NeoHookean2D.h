/* $Id: RG_NeoHookean2D.h,v 1.1 2002-10-04 23:59:01 thao Exp $ */
/* created: TDN (01/22/2001) */

#ifndef _RG_NeoHookean2D_
#define _RG_NeoHookean2D_

#include "RG_VDSplit2D.h"

namespace Tahoe {

class RG_NeoHookean2D: public RG_VDSplit2D
{
  public:

	/* constructor */
	RG_NeoHookean2D(ifstreamT& in, const FiniteStrainT& element);

	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;
	virtual void Initialize(void);
  protected:
	virtual double Phi(const dArrayT& eigenstretch_bar, const double& J,
			   const int SpringType);
	virtual void devTau(const dArrayT& eigenstretch_bar, 
			    dArrayT& eigenstress, const int SpringType);
	virtual double meanTau(const double& J, const int SpringType);
	virtual void DdevTauDepsilon(const dArrayT& eigenstretch_bar, 
				     dSymMatrixT&  eigenmodulus, 
				     const int SpringType);
	virtual double DmeanTauDepsilon(const double& J, const int SpringType);
	virtual void OutOfPlaneStretch(const dArrayT& eigenstretch_bar, 
				       const double& J, const int SpringType);

  private:  
  	/*inelastic moduli*/
	dArrayT fMu;
	dArrayT fKappa;

	/*out of plane strain*/
	double fl2_bar;
};
}
#endif /* _RG_NeoHookean2D_ */
