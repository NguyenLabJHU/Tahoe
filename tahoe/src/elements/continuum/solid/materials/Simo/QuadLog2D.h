/* $Id: QuadLog2D.h,v 1.2.6.1 2002-06-27 18:03:16 cjkimme Exp $ */
/* created: paklein (06/28/1997)                                          */
/* (2D <-> 3D) translator for the QuadLog3D.                              */

#ifndef _QUAD_LOG_2D_
#define _QUAD_LOG_2D_

/* base classes */
#include "QuadLog3D.h"
#include "Material2DT.h"


namespace Tahoe {

class QuadLog2D: public QuadLog3D, public Material2DT
{
public:

	/* constructor */
	QuadLog2D(ifstreamT& in, const FiniteStrainT& element);

	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;

	/* modulus */
	virtual const dMatrixT& c_ijkl(void);
	
	/* stress */
	virtual const dSymMatrixT& s_ij(void);

	/* strain energy density */
	virtual double StrainEnergyDensity(void);

protected:

	/* return values */
	dSymMatrixT fStress2D;
	dMatrixT    fModulus2D;

	/* workspace */
	dSymMatrixT fb_2D;
};

} // namespace Tahoe 
#endif /* _QUAD_LOG_2D_ */
