/* $Id: NL_E_Mat2DT.h,v 1.3.8.1 2002-10-28 06:49:27 paklein Exp $ */
/* created: paklein (06/13/1997) */
#ifndef _NL_E_MAT_2D_T_H_
#define _NL_E_MAT_2D_T_H_

/* base classes */
#include "NL_E_MatT.h"
#include "Material2DT.h"

namespace Tahoe {

/** base class for materials with 2D nonlinear elastic behavior.
 * See documentation for NL_E_MatT.
 * \note The 2D deformation constraint is assumed to be prescribed
 * by the derived material classes, ie. it is not specified
 * by an input file parameter.
 */
class NL_E_Mat2DT: public NL_E_MatT, public Material2DT
{
public:

	/* constructor */
	NL_E_Mat2DT(ifstreamT& in, const FDMatSupportT& support, ConstraintOptionT constraint);

	/* print parameters */
	virtual void Print(ostream& out) const;
	
	/* modulus */
	virtual const dMatrixT& c_ijkl(void);
	
	/* stress */
	virtual const dSymMatrixT& s_ij(void);

	/* strain energy density */
	virtual double StrainEnergyDensity(void);

protected:

	/* symmetric Cij reduced index matrix */
	virtual void ComputeModuli(const dSymMatrixT& E, dMatrixT& moduli) = 0;	
	
	/* symmetric 2nd Piola-Kirchhoff reduced index vector */
	virtual void ComputePK2(const dSymMatrixT& E, dSymMatrixT& PK2) = 0;

	/* strain energy density for the specified strain */
	virtual double ComputeEnergyDensity(const dSymMatrixT& E) = 0;
	
};

} // namespace Tahoe 
#endif /* _NL_E_MAT_2D_T_H_ */
