/* $Id: FBC_ControllerT.h,v 1.6 2002-11-14 17:06:46 paklein Exp $ */
/* created: paklein (11/17/1997) */

#ifndef _FBC_CONTROLLER_T_H_
#define _FBC_CONTROLLER_T_H_

#include "Environment.h"
#include "GlobalT.h"

#include "ios_fwd_decl.h"

namespace Tahoe {

/* forward declarations */
class ifstreamT;
class FEManagerT;
class SolverT;
template <class TYPE> class AutoArrayT;
class iArray2DT;
template <class TYPE> class RaggedArray2DT;
class eControllerT;
class StringT;

/** base class for all force BC controllers */
class FBC_ControllerT
{
public:

	/* controller codes - derived classes */
	enum CodeT {kPenaltyWall = 0,
	          kPenaltySphere = 1,
               kAugLagSphere = 2,
            kMFPenaltySphere = 3,
                 kAugLagWall = 4};

	/* constructor */
	FBC_ControllerT(FEManagerT& fe_manager, int group);

	/* destructor */
	virtual ~FBC_ControllerT(void);

	/* set the controller */
	virtual void SetController(const eControllerT* controller);

	/* initialize data - called immediately after construction */
	virtual void Initialize(void) = 0;

	/* form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const = 0;

	/* nodally generated DOF's and tags (pseudo nodes) */
	/* append element equations numbers to the list */
	virtual void Equations(AutoArrayT<const iArray2DT*>& eq_1,
		AutoArrayT<const RaggedArray2DT<int>*>& eq_2);
	virtual void Connectivities(AutoArrayT<const iArray2DT*>& connects_1,
		AutoArrayT<const RaggedArray2DT<int>*>& connects_2) const;

	/* initial condition/restart functions
	 *
	 * Set to initial conditions.  The restart functions
	 * should read/write any data that overrides the default
	 * values */
	virtual void InitialCondition(void) = 0;
	virtual void ReadRestart(istream& in);
	virtual void WriteRestart(ostream& out) const;

	/* apply force and tangent contributions */
	virtual void ApplyLHS(void) = 0;
	virtual void ApplyRHS(void) = 0;

	/* initialize/finalize step */
	virtual void InitStep(void) = 0;
	virtual void CloseStep(void) = 0;

	/* reset to the last known solution */
	virtual void Reset(void) = 0;

	/* output current configuration */
	virtual void WriteOutput(ostream& out) const = 0;

	/* input processing */
	virtual void EchoData(ifstreamT& in, ostream& out) = 0;
	
protected:

	/** the Boss */
	FEManagerT& fFEManager;

	/** equation group */
	int fGroup;

	/* element controller */
	const eControllerT* fController;
};

} // namespace Tahoe 
#endif /* _FBC_CONTROLLER_T_H_ */
