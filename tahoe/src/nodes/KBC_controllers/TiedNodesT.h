/* $Id: TiedNodesT.h,v 1.21 2004-07-15 08:31:21 paklein Exp $ */
#ifndef _TIED_NODES_T_H_
#define _TIED_NODES_T_H_

/* base class */
#include "KBC_ControllerT.h"

/* direct members */
#include "dArray2DT.h"
#include "iArray2DT.h"
#include "iArrayT.h"
#include "ScheduleT.h"
#include "AutoArrayT.h"

namespace Tahoe {

/* forward declarations */
class dArray2DT;
class BasicFieldT;
class FEManagerT;
class SurfacePotentialT;
class TiedPotentialBaseT;
} 

namespace Tahoe {

/** class to tie nodes together and release them under specified conditions.
 * The class accomplishes this in the following way, most of which rely on the
 * fact that KBC_ControllerT's act on the system before the elements do, as
 * determined by the calling conventions in FEManagerT:
 * <ul> 
 * <li> paired nodes are returned as connectivities in TiedNodesT::Connects
 *      so that the graph for the mesh reflects the tied condition.
 * <li> KBC_CardT's are generated during the call to TiedNodesT::Initialize for each 
 *      degree of freedom of the \e follower nodes to reflect that these degrees 
 *      of freedom are not active. The cards are reset during TiedNodesT::RelaxSystem
 *      if the status of any node pair changes.
 * <li> The equation numbers from the \e leader nodes are copied into the equations of
 *      the follower node during the call to TiedNodesT::Equations, so the global
 *      equation system reflects the tied conditions.
 * <li> The kinematic quantities registered with TiedNodesT::AddKinematics are copied
 *      from the leader nodes to the follower nodes during calls to TiedNodesT::FormRHS,
 *      TiedNodesT::RelaxSystem, and TiedNodesT::CloseStep.
 * <li> The tied constraints can be tested and released during the call to
 *      TiedNodesT::RelaxSystem. Specifically, tests should be added to the
 *      TiedNodesT::ChangeStatus function. 
 * </ul> */
class TiedNodesT: public KBC_ControllerT
{
public:	

	/** constructor */
	TiedNodesT(const BasicSupportT& support, BasicFieldT& field);

	/** initialize directly instead of using TiedNodesT::Initialize */
	virtual void SetTiedPairs(iArrayT& follower, iArrayT& leader);

	/** inform controller of external nodes */
	virtual void SetExternalNodes(const ArrayT<int>& ex_nodes) const;

	/** set to initial conditions. Reset all conditions to tied. */
	virtual void InitialCondition(void);

	/** \name restart functions */
	/*@{*/
	virtual void ReadRestart(istream& in);
	virtual void WriteRestart(ostream& out) const;
	/*@}*/

	/** \name solution steps
	 * Methods signaling different stages of the solution process for
	 * a single time step. */
	/*@{*/
	/** initialize the current step */
	virtual void InitStep(void);
	
	/** computing residual force */
	virtual void FormRHS(void);

	/** signal that the solution has been found */
	virtual void CloseStep(void);

	/** solution for the current step failed. Restore system to its
	 * state at the beginning of the current time step. */
	virtual void Reset(void);
	/*@}*/

	/** see if pair status has changed.
	 * \return GlobalT::kReEQ if any pair status has changed. */
	virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/** append connectivities generated by the controller. By default, 
	 * does not append any connectivities to the list */
	virtual void Connectivities(AutoArrayT<const iArray2DT*>& connects, 
		AutoArrayT<const iArray2DT*>& equivalent_nodes) const;

	/** append equation number sets generated by the controller */
	virtual void Equations(AutoArrayT<const iArray2DT*>& equations) const;

	virtual void SetEquations(void);

protected:

	/** check status of pairs.
	 * \return true if the status of any pair has changed */
	virtual bool ChangeStatus(void);

	/** pair status */
	enum StatusT {
		kTied = 1, /**< nodes move together */
		kFree = 0,  /**< nodes are independent */
		kChangeF = 2, /**< follower node is external and should be deleted */
		kChangeL = 3, /**< leader node is external and should be deleted*/
	  	kTiedExt = 4 /**< pair is external and musn't be released*/
	};

	/** generate boundary condition card for each degree of freedom
	 * of follower nodes with TiedNodesT::kTied status */
	void SetBCCards(void);
	
	/** apply the update to the solution. Does nothing by default. */
	virtual void Update(const dArrayT& update);

	/** copy kinematic information from the leader nodes to the follower nodes */
	virtual void CopyKinematics(void);

	/** output current configuration */
	virtual void WriteOutput(ostream& out) const;

	/** \name called by TiedNodesT::Initialize */
	/*@{*/
	/** read class parameters. Called after nodes information is read.
	 * TiedNodesT::ReadParameters does nothing. */
	virtual void ReadParameters(ifstreamT&) { };

	/** set initial tied node pairs. Initializes the data in TiedNodesT::fLeaderIds,
	 * TiedNodesT::fFollowerIds, TiedNodesT::fNodePairs, and TiedNodesT::fPairStatus.
	 * TiedNodesT::InitTiedNodePairs pairs nodes that coincide. */
	virtual void InitTiedNodePairs(const iArrayT& leader, iArrayT& follower);
	/*@}*/

protected:

	/** the field */
	BasicFieldT& fField;

	/** the tied node pairs */
	/*@{*/
	/** id list for the \e leader node sets */
	ArrayT<StringT> fLeaderIds;

	/** id list for the \e leader node sets */
	ArrayT<StringT> fFollowerIds;
	
	/** first node is \e follower second node is \e leader or -1 if no
	 * leader is found. */
	iArray2DT fNodePairs;	

	/** status of the pair in terms of TiedNodesT::StatusT */
	iArrayT fPairStatus;

	/** status history */
	iArrayT fPairStatus_last;
	/*@}*/
	
	/** needed to generate KBC_ControllerT::fKBC_Cards */
	ScheduleT fDummySchedule;	

	/* false if TiedPotentialT cohesive law is being used 
	 * to determine release condition
	 */
	bool qNoTiedPotential;
	/** Element Groups to get updated list of free nodes from */
	iArrayT iElemGroups;

};

} // namespace Tahoe 
#endif /* _TIED_NODES_T_H_ */
