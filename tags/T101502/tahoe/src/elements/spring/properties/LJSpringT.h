/* $Id: LJSpringT.h,v 1.3 2002-07-05 22:28:28 paklein Exp $ */
/* created: paklein (11/20/1996)                                          */

#ifndef _LJ_SPRINGT_H_
#define _LJ_SPRINGT_H_

/* base class */
#include "RodMaterialT.h"

namespace Tahoe {

/* forward declarations */
class ThermalDilatationT;
class ElementBaseT;

class LJSpringT: public RodMaterialT
{
public:

	/* constructor */
	LJSpringT(ifstreamT& in);

	/* returns trues TRUE since the initial length is always assumed
	 * to be non-equilibrium */
	virtual int HasInternalStrain(void) const;
	
	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;
	
	/* potential function and derivatives */
	virtual double Potential(double rmag, double Rmag) const;
	virtual double DPotential(double rmag, double Rmag) const;
	virtual double DDPotential(double rmag, double Rmag) const;
	
private:

	double	fLJConstant;
	
};

} // namespace Tahoe 
#endif /* _LJ_SPRINGT_H_ */