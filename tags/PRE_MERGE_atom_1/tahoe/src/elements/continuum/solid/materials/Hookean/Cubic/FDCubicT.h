/* $Id: FDCubicT.h,v 1.4 2002-11-14 17:06:04 paklein Exp $ */
/* created: paklein (06/11/1997) */
#ifndef _FD_CUBIC_T_H_
#define _FD_CUBIC_T_H_

/* base classes */
#include "FDHookeanMatT.h"
#include "CubicT.h"

namespace Tahoe {

class FDCubicT: public FDHookeanMatT, public CubicT
{
public:

	/* constructor */
	FDCubicT(ifstreamT& in, const FDMatSupportT& support);

	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;

protected:

	/* set (material) tangent modulus */
	virtual void SetModulus(dMatrixT& modulus);
};

} // namespace Tahoe 
#endif /* _FD_CUBIC_T_H_ */