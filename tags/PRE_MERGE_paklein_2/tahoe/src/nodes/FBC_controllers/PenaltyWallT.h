/* $Id: PenaltyWallT.h,v 1.6 2002-07-02 19:56:28 cjkimme Exp $ */
/* created: paklein (02/25/1997) */

#ifndef _PENATLY_WALL_T_H_
#define _PENATLY_WALL_T_H_

/* base class */
#include "PenaltyRegionT.h"

/* direct members */
#include "ElementMatrixT.h"


namespace Tahoe {

class PenaltyWallT: public PenaltyRegionT
{
public:

	/* constructor */
	PenaltyWallT(FEManagerT& fe_manager, int group, const iArray2DT& eqnos,
		const dArray2DT& coords, const dArray2DT* vels);

	/* input processing */
	virtual void EchoData(ifstreamT& in, ostream& out);

	/* initialize data */
	virtual void Initialize(void);

	/* tangent */
	virtual void ApplyLHS(void);

private:

	/* accumulate the contact force vector fContactForce */
	virtual void ComputeContactForce(double kforce);

protected:

	/* wall parameters */
	dArrayT 	fnormal;
//	double fmu;    //coefficient of friction
	
	/* wall normal and tangents */
	dArrayT		fntforce;
	dArrayT		fxyforce;
	dMatrixT	fQ;

	/* relative displacements */
	dArray2DT	fp_i; //relative displacement
	dArray2DT	fv_i; //relative velocity

	/* workspace */
	ElementMatrixT fLHS;  //tangent matrix
	dArrayT        fd_sh; //shallow
	iArrayT        fi_sh; //shallow
};

} // namespace Tahoe 
#endif /* _PENATLY_WALL_T_H_ */
