/* $Id: ScaledVelocityNodesT.h,v 1.3.38.2 2004-05-26 18:09:43 paklein Exp $ */
#ifndef _SCALED_VELOCITY_NODES_T_H_
#define _SCALED_VELOCITY_NODES_T_H_

/* base class */
#include "KBC_ControllerT.h"

/* direct members */
#include "ScheduleT.h"
#include "iArrayT.h"
#include "BasicFieldT.h"

namespace Tahoe {

/* forward declarations */
class BasicFieldT;
class RandomNumberT;

/** Nodes whose velocities are scaled to have zero net momentum
 * and kinetic energies that sum to 3/2 N k_B T
 */
class ScaledVelocityNodesT: public KBC_ControllerT
{
public:	

	/** constructor */
	ScaledVelocityNodesT(NodeManagerT& node_manager, BasicFieldT& field);

	/** initialize data. Must be called immediately after construction */
	virtual void Initialize(ifstreamT& in);

	/** do at start of timestep */
	virtual void InitStep(void);

	/** Initialize to appropriate temperature */
	virtual void InitialCondition(void);
	
	virtual bool IsICController(void) { return true; }

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** describe the parameters needed by the interface */
	virtual void DefineParameters(ParameterListT& list) const;
	
	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& list_name) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

protected:
	
	void SetBCCards(void);

protected:

	/** the field */
	BasicFieldT& fField;

	/** True if controller only used for IC */
	bool qIConly;
	
	/** flag to let this controller only influence ICs */
	bool qFirstTime;
	
	/** true if allNodes needs to be initialized or rescaled */
	bool qAllNodes;

	/** rescale every fIncs timesteps */
	int fIncs, fIncCt;

	/** temperature evolution controlled by a schedule */
	const ScheduleT* fTempSchedule;
	int fnumTempSchedule;
	double fTempScale;
	
	/** temperature schedule is not the BC value. Need a dummy schedule, too */
	ScheduleT fDummySchedule;
	
	/** the tied node pairs */
	/*@{*/
	/** id list for the \e leader node sets */
	ArrayT<StringT> fNodeIds;
	iArrayT fNodes;

	/** assuming all nodes have same mass */
	double fMass;
	
	/** initial velocity distribution random number gen */
	RandomNumberT fRandom;

	/** initial temperature */
	double fT_0;
};

} // namespace Tahoe 
#endif /* _SCALED_VELOCITY_NODES_T_H_ */
