/* $Id: PolyBasis1DT.h,v 1.1.1.1.10.1 2002-06-27 18:04:13 cjkimme Exp $ */
/* created: paklein (12/11/1999)                                          */
/* base class for basis functions                                         */

#ifndef _POLYBASIS_1D_T_H_
#define _POLYBASIS_1D_T_H_

/* base class */
#include "BasisT.h"


namespace Tahoe {

class PolyBasis1DT: public BasisT
{
public:

	/* constructor */
	PolyBasis1DT(int complete);
	
	/* return the number of basis functions */
	virtual int BasisDimension(void) const;

	/* evaluate basis functions at coords */
	virtual void SetBasis(const dArray2DT& coords, int order);

};

} // namespace Tahoe 
#endif /* _POLYBASIS_1D_T_H_ */
