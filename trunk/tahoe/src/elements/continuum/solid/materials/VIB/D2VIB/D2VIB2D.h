/* $Id: D2VIB2D.h,v 1.2 2002-07-02 19:55:59 cjkimme Exp $ */
/* created: paklein (10/23/1999)                                          */

#ifndef _D2_VIB_2D_H_
#define _D2_VIB_2D_H_

/* base class */
#include "VIB2D.h"

/* forward declarations */

namespace Tahoe {

class D2MeshFreeFDElasticT;
class D2MeshFreeShapeFunctionT;

class D2VIB2D: public VIB2D
{
public:

	/* constructor */
	D2VIB2D(ifstreamT& in, const D2MeshFreeFDElasticT& element);

	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;	

	/* material internal stress terms */
	virtual void StressTerms(dMatrixT& DW, dMatrixT& DDW) = 0;

	/* DISABLE */
//	virtual const dMatrixT& c_ijkl(void);  // spatial tangent moduli
//	virtual const dSymMatrixT& s_ij(void); // Cauchy stress
//	virtual const dMatrixT& C_IJKL(void);  // material tangent moduli
//	virtual const dSymMatrixT& S_IJ(void); // PK2 stress

protected:

	/* higher order gradient shape functions */
	const D2MeshFreeShapeFunctionT& fD2MLSShape;

	/* length scale parameter (squared) */
	double feps2;
};

} // namespace Tahoe 
#endif /* _D2_VIB_2D_H_ */
