/* $Id: D2MeshFreeShapeFunctionT.h,v 1.2 2001-06-19 23:22:05 paklein Exp $ */
/* created: paklein (10/23/1999)                                          */

#ifndef _D2_MF_SHAPE_T_H_
#define _D2_MF_SHAPE_T_H_

/* base class */
#include "MeshFreeShapeFunctionT.h"

/* forward declarations */
class D2MeshFreeSupportT;

class D2MeshFreeShapeFunctionT: public MeshFreeShapeFunctionT
{
public:

/* constructors */
	D2MeshFreeShapeFunctionT(GeometryT::CodeT geometry_code, int numIP,
		const LocalArrayT& coords, const dArray2DT& all_coords,
		const iArray2DT& connects, const iArrayT& nongridnodes,
		const int& currelement, ifstreamT& in);

	/* compute global shape derivatives */ 	
	virtual void SetDerivatives(void);
	int SetDerivativesAt(const dArrayT& x, AutoArrayT<int>& nodes);
		// returns 0 if MLS fit fails

	/* 2nd order shape function gradients matrix */
	void D2GradNa(dMatrixT& D2_grad_Na) const;

	/* 2nd order spatial gradients */
	void GradGradU(const LocalArrayT& nodal, dMatrixT& gradgrad_U) const;

	/* reconstruct displacement field and all derivatives */
	void NodalField(const dArray2DT& DOF, dArray2DT& field, dArray2DT& Dfield,
		dArray2DT& DDfield, iArrayT& nodes);

protected:

	/* meshfree database support */
	D2MeshFreeSupportT* fD2MFSupport;

	ArrayT<dArray2DT> fDDNaU;
	
	/* work space for blended shape functions */
	ArrayT<dArray2DT> fDDNa_tmp;
};

/* inlines */

/* spatial gradients */
inline void D2MeshFreeShapeFunctionT::GradGradU(const LocalArrayT& nodal,
	dMatrixT& gradgrad_U) const
{
	fDomain->Jacobian(nodal, fDDNaU[fCurrIP], gradgrad_U);	
}

#endif /* _D2_MF_SHAPE_T_H_ */
