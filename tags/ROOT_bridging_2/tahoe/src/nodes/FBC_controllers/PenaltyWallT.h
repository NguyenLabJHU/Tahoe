/* $Id: PenaltyWallT.h,v 1.7 2002-11-28 16:44:20 paklein Exp $ */
/* created: paklein (02/25/1997) */

#ifndef _PENATLY_WALL_T_H_
#define _PENATLY_WALL_T_H_

/* base class */
#include "PenaltyRegionT.h"

/* direct members */
#include "ElementMatrixT.h"


namespace Tahoe {

/** flat rigid barrier enforced with a penalized constraint */
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

	/** tangent
	 * \param sys_type "maximum" tangent type needed by the solver. The GlobalT::SystemTypeT
	 *        enum is ordered by generality. The solver should indicate the most general
	 *        system type that is actually needed. */
	virtual void ApplyLHS(GlobalT::SystemTypeT sys_type);

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
