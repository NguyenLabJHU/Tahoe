/* $Id: SimoIso2D.h,v 1.3.6.1 2002-06-27 18:03:17 cjkimme Exp $ */
/* created: paklein (03/04/1997) */

#ifndef _SIMO_ISO_2D_H_
#define _SIMO_ISO_2D_H_

/* base classes */
#include "SimoIso3D.h"
#include "Material2DT.h"

/** (2D <-> 3D) translator for the SimoIso3D */

namespace Tahoe {

class SimoIso2D: public SimoIso3D, public Material2DT
{
public:

	/** constructor */
	SimoIso2D(ifstreamT& in, const FiniteStrainT& element);

	/** print parameters */
	virtual void Print(ostream& out) const;

	/** print material model name */
	virtual void PrintName(ostream& out) const;

	/** initialize step. Verify that the thermal dilatation deformation
	 * gradient is equibiaxial. This state is assumed when computing
	 * the state of plain strain deformation. */
	virtual void InitStep(void);

	/** spatial tangent modulus */
	virtual const dMatrixT& c_ijkl(void);
	
	/** Cauchy stress */
	virtual const dSymMatrixT& s_ij(void);

	/** strain energy density */
	virtual double StrainEnergyDensity(void);

protected:

	/** compute 3D stretch tensor \b b from the 2D deformation state. 
	 * \todo Make this a FDStructMatT function? */
	void Compute_b_3D(dSymMatrixT& b_3D);

protected:

	/* return values */
	dSymMatrixT fStress2D;  /**< return value for the Cauchy stress */
	dMatrixT    fModulus2D; /**< return value for the spatial tangent modulus */
	
	/** workspace */
	dSymMatrixT fb_2D;		 	 	
};

} // namespace Tahoe 
#endif /* _SIMO_ISO_2D_H_ */
