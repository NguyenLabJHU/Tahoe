/* $Id: LJSpringT.h,v 1.1.1.1.10.1 2002-06-27 18:03:54 cjkimme Exp $ */
/* created: paklein (11/20/1996)                                          */

#ifndef _LJ_SPRINGT_H_
#define _LJ_SPRINGT_H_

/* base class */
#include "RodMaterialT.h"

/* forward declarations */

namespace Tahoe {

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
