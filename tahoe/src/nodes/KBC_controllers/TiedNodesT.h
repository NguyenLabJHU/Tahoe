/* $Id: TiedNodesT.h,v 1.5.2.2 2002-04-24 01:29:27 paklein Exp $ */

#ifndef _TIED_NODES_T_H_
#define _TIED_NODES_T_H_

/* base class */
#include "KBC_ControllerT.h"

/* direct members */
#include "iArray2DT.h"
#include "iArrayT.h"
#include "ScheduleT.h"
#include "AutoArrayT.h"

/* forward declarations */
class dArray2DT;
class FEManagerT;

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
	TiedNodesT(NodeManagerT& node_manager);

	/** initialize data. Must be called immediately after construction */
	virtual void Initialize(ifstreamT& in);
	virtual void WriteParameters(ostream& out) const;

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
	virtual void Connectivities(AutoArrayT<const iArray2DT*>& connects) const;

	/** append equation number sets generated by the controller */
	virtual void Equations(AutoArrayT<const iArray2DT*>& equations) const;

	/** \name TiedNodesT extras
	 * extra information needed by this class */
	/*@{*/	 
	void SetEquations(iArray2DT& eqnos) { fEqnos = &eqnos; };
	void AddKinematics(dArray2DT& u);
	/*@}*/	 

protected:

	/** check status of pairs.
	 * \return true if the status of any pair has changed */
	virtual bool ChangeStatus(void);

//private:

	/** pair status */
	enum StatusT {
		kTied = 1, /**< nodes move together */
		kFree = 0  /**< nodes are independent */};

	/** generate boundary condition card for each degree of freedom
	 * of follower nodes with TiedNodesT::kTied status */
	void SetBCCards(void);

	/** copy kinematic information from the leader nodes to the follower nodes */
	virtual void CopyKinematics(void);

protected:

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
	
	/** equations numbers of the global system */
	iArray2DT* fEqnos;

	/** list of kinematics for the nodes */
	AutoArrayT<dArray2DT*> fKinematics;
	
	/** needed to generate KBC_ControllerT::fKBC_Cards */
	ScheduleT fDummySchedule;	

	const FEManagerT& fFEManager;
};

#endif /* _TIED_NODES_T_H_ */
