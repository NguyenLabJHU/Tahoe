/* $Id: FDCubic2DT.h,v 1.6.30.2 2004-03-02 17:46:14 paklein Exp $ */
/* created: paklein (06/11/1997) */
#ifndef _FD_CUBIC_2D_T_H_
#define _FD_CUBIC_2D_T_H_

/* base classes */
#include "FDCubicT.h"
#include "Anisotropic2DT.h"

namespace Tahoe {

class FDCubic2DT: public FDCubicT, public Anisotropic2DT
{
public:

	/** constructor */
	FDCubic2DT(ifstreamT& in, const FSMatSupportT& support);
	FDCubic2DT(void);

	/* print parameters */
	virtual void Print(ostream& out) const;

	/** return the pressure associated with the last call to 
	 * SolidMaterialT::s_ij. See SolidMaterialT::Pressure
	 * for more information. \note plane strain not implemented, but 
	 * could be using CubicT::DilatationFactor2D. */
	virtual double Pressure(void) const;

protected:

	/** set modulus */
	virtual void SetModulus(dMatrixT& modulus);

private:

	/** set inverse of thermal transformation - return true if active */
	virtual bool SetInverseThermalTransformation(dMatrixT& F_trans_inv);  			
};

} /* namespace Tahoe */

#endif /* _FD_CUBIC_2D_T_H_ */
