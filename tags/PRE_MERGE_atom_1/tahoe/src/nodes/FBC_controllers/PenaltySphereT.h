/* $Id: PenaltySphereT.h,v 1.4 2002-11-28 16:44:20 paklein Exp $ */
/* created: paklein (04/30/1998) */

#ifndef _PENATLY_SPHERE_T_H_
#define _PENATLY_SPHERE_T_H_

/* base class */
#include "PenaltyRegionT.h"

/* direct members */
#include "ElementMatrixT.h"

namespace Tahoe {

/** spherical rigid barrier enforced with a penalized constraint */
class PenaltySphereT: public PenaltyRegionT
{
public:

	/* constructor */
	PenaltySphereT(FEManagerT& fe_manager, int group, const iArray2DT& eqnos, 
		const dArray2DT& coords, const dArray2DT* vels);

	/* input processing */
	virtual void EchoData(ifstreamT& in, ostream& out);

	/* initialize data */
	virtual void Initialize(void);

	/* form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/** tangent
	 * \param sys_type "maximum" tangent type needed by the solver. The GlobalT::SystemTypeT
	 *        enum is ordered by generality. The solver should indicate the most general
	 *        system type that is actually needed. */
	virtual void ApplyLHS(GlobalT::SystemTypeT sys_type);
	
protected:

	/* accumulate the contact force vector fContactForce */
	virtual void ComputeContactForce(double kforce);

protected:

	/* sphere radius */
	double fRadius;
	
	/* center to striker distances */
	dArrayT	fDistances;
	
	/* workspace */
	dArrayT        fv_OP; //vector from center to contact node
	ElementMatrixT fLHS;  //tangent matrix
	dArrayT        fd_sh; //shallow
	iArrayT        fi_sh; //shallow
};

} // namespace Tahoe 
#endif /* _PENATLY_SPHERE_T_H_ */