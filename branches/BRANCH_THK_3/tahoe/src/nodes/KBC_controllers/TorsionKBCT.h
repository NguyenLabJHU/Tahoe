/* $Id: TorsionKBCT.h,v 1.2 2003-05-31 18:38:29 paklein Exp $ */
#ifndef _TORSION_KBC_T_H_
#define _TORSION_KBC_T_H_

/* base class */
#include "KBC_ControllerT.h"

/* direct members */
#include "iArrayT.h"
#include "dArrayT.h"
#include "ScheduleT.h"

namespace Tahoe {

/** torsion boundary condition controller. */
class TorsionKBCT: public KBC_ControllerT
{
public:

	/** constructor */
	TorsionKBCT(NodeManagerT& node_manager, const double& time);

	/** read parameters from stream */
	virtual void Initialize(ifstreamT& in);

	/** set initial conditions */
	void InitialCondition(void);

	/** write parameters to output stream */
	virtual void WriteParameters(ostream& out) const;

	/** compute updated prescibed displacements */
	virtual void InitStep(void);

protected:

	/** time increment */
	const double& fTime;
	double fStartTime;

	/** rotation rate (rad/s) */
	double fw;
	
	/** \name axis of rotation */
	/*@{*/
	/** direction */
	int fAxis;
	
	/** point on the axis */
	dArrayT fPoint;
	/*@}*/

	/** \name nodes */
	/*@{*/
	ArrayT<StringT> fID_List;
	iArrayT fNodes;
	/*@}*/

	/** runtime data */
	ScheduleT fDummySchedule;
};

} /* namespace Tahoe */

#endif /* _TORSION_KBC_T_H_ */