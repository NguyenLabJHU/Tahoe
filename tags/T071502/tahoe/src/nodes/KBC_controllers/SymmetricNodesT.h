/* $Id: SymmetricNodesT.h,v 1.4 2002-07-05 22:28:31 paklein Exp $ */

#ifndef _SYMMETRIC_NODES_T_H_
#define _SYMMETRIC_NODES_T_H_

/* base class */
#include "TiedNodesT.h"

/* direct members */
#include "dArray2DT.h"

namespace Tahoe {

/* forward declarations */
class dArray2DT;
class FEManagerT;

/** class derived from TiedNodesT. Intended to provide a means to enforce
 * symmetry constraints along the line(plane) in 2D(3D) determined by 
 * follower and leader nodes. This symmetry is enforced in function
 * CopyKinematics. */
class SymmetricNodesT: public Tahoe::TiedNodesT
{
public:	

	/** constructor */
	SymmetricNodesT(NodeManagerT& node_manager, BasicFieldT& field);

	/** initialize data. Must be called immediately after construction */
       	virtual void Initialize(ifstreamT& in);
	//virtual void WriteParameters(ostream& out) const;

	/** set to initial conditions. Reset all conditions to tied. */
	//virtual void InitialCondition(void);

	/** \name restart functions */
	/*@{*/
	//virtual void ReadRestart(istream& in);
	//virtual void WriteRestart(ostream& out) const;
	/*@}*/

	/** \name solution steps
	 * Methods signaling different stages of the solution process for
	 * a single time step. */
	/*@{*/
	/** initialize the current step */
	//virtual void InitStep(void);
	
	/** computing residual force */
	//virtual void FormRHS(void);

	/** signal that the solution has been found */
	//virtual void CloseStep(void);

	/** solution for the current step failed. Restore system to its
	 * state at the beginning of the current time step. */
	//virtual void Reset(void);
	/*@}*/

	/** see if pair status has changed.
	 * \return GlobalT::kReEQ if any pair status has changed. */
	//virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/** append connectivities generated by the controller. By default, 
	 * does not append any connectivities to the list */
	//virtual void Connects(AutoArrayT<const iArray2DT*>& connects) const;

	/** append equation number sets generated by the controller */
	//virtual void Equations(AutoArrayT<const iArray2DT*>& equations) const;

	/** \name TiedNodesT extras
	 * extra information needed by this class */
	/*@{*/	 
	//void SetEquations(iArray2DT& eqnos) { fEqnos = &eqnos; };
	//void AddKinematics(dArray2DT& u);
	/*@}*/	 

protected:

	/** check status of pairs.
	 * \return true if the status of any pair has changed */
	virtual bool ChangeStatus(void);

private:

	/** generate boundary condition card for each degree of freedom
	 * of follower nodes with TiedNodesT::kTied status */
	//void SetBCCards(void);

	/** copy kinematic information from the leader nodes to the follower nodes */
	//virtual void CopyKinematics(void);

protected:

	/** the tied node pairs */
	/*@{*/
	/** id list for the \e leader node sets */
	//ArrayT<StringT> fLeaderIds;

	/** id list for the \e leader node sets */
	//ArrayT<StringT> fFollowerIds;
	
	/** first node is \e follower second node is \e leader or -1 if no
	 * leader is found. */
	//iArray2DT fNodePairs;	

	/** status of the pair in terms of TiedNodesT::StatusT */
	//iArrayT fPairStatus;

	/** status history */
	//iArrayT fPairStatus_last;
	/*@}*/
	
	/** equations numbers of the global system */
	//iArray2DT* fEqnos;

	/** list of kinematics for the nodes */
	//AutoArrayT<dArray2DT*> fKinematics;
	
	/** needed to generate KBC_ControllerT::fKBC_Cards */
	//LoadTime fDummySchedule;	

	//const FEManagerT& fFEManager;
 private:

	dArray2DT fDir;
	
};

} // namespace Tahoe 
#endif /* _SYMMETRIC_NODES_T_H_ */
