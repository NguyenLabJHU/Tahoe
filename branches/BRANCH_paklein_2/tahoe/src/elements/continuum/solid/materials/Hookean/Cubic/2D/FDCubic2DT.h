/* $Id: FDCubic2DT.h,v 1.4 2002-10-05 20:04:11 paklein Exp $ */
/* created: paklein (06/11/1997) */
#ifndef _FD_CUBIC_2D_T_H_
#define _FD_CUBIC_2D_T_H_

/* base classes */
#include "FDCubicT.h"
#include "Anisotropic2DT.h"
#include "Material2DT.h"

namespace Tahoe {

class FDCubic2DT: public FDCubicT, public Anisotropic2DT, public Material2DT
{
public:

	/* constructor */
	FDCubic2DT(ifstreamT& in, const FiniteStrainT& element);

	/* print parameters */
	virtual void Print(ostream& out) const;

	/** return the pressure associated with the last call to 
	 * StructuralMaterialT::s_ij. See StructuralMaterialT::Pressure
	 * for more information. \note plane strain not implemented, but 
	 * could be using CubicT::DilatationFactor2D. */
	virtual double Pressure(void) const;

protected:

	/* set modulus */
	virtual void SetModulus(dMatrixT& modulus);
	
private:

	/* set inverse of thermal transformation - return true if active */
	virtual bool SetInverseThermalTransformation(dMatrixT& F_trans_inv);  			
};

} // namespace Tahoe 
#endif /* _FD_CUBIC_2D_T_H_ */
