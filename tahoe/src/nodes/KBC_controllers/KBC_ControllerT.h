/* $Id: KBC_ControllerT.h,v 1.7 2002-07-02 19:56:35 cjkimme Exp $ */
/* created: paklein (09/05/2000) */

#ifndef _KBC_CONTROLLER_T_H_
#define _KBC_CONTROLLER_T_H_

#include "Environment.h"
#include "GlobalT.h"

/* direct members */
#include "ArrayT.h"
#include "KBC_CardT.h"

/* forward declarations */
#include "ios_fwd_decl.h"

namespace Tahoe {

class ifstreamT;
class NodeManagerT;
class nControllerT;
class iArrayT;
class StringT;
class dArrayT;
class iArray2DT;
template <class TYPE> class AutoArrayT;

/** base class for all kinematic BC controllers. Classes that
 * implement more than simple boundary conditions */
class KBC_ControllerT
{
public:

	/** controller codes - derived classes */
	enum CodeT {kK_Field = 0,
      kBimaterialK_Field = 1,
         kMappedPeriodic = 2,
              kTiedNodes = 3,
         kSymmetricNodes = 4};

	/** constructor */
	KBC_ControllerT(NodeManagerT& node_manager);

	/** destructor */
	virtual ~KBC_ControllerT(void);

	/** boundary condition cards generated by the controller */
	const ArrayT<KBC_CardT>& KBC_Cards(void) const;

	/** initialize data. Must be called immediately after construction */
	virtual void Initialize(ifstreamT& in) = 0;
	virtual void WriteParameters(ostream& out) const;

	/** set to initial conditions */
	virtual void InitialCondition(void) = 0;

	/** \name restart functions */
	/*@{*/
	virtual void ReadRestart(istream& in);
	virtual void WriteRestart(ostream& out) const;
	/*@}*/

	/** \name solution steps
	 * Methods signalling different stages of the solution process for
	 * a single time step. */
	/*@{*/
	/** initialize the current step */
	virtual void InitStep(void) { };

	/** computing residual force */
	virtual void FormRHS(void) { };

	/** computing tangent */
	virtual void FormLHS(void) { };
	
	/** apply the update to the solution. Does nothing by default. */
	virtual void Update(const dArrayT& update);

	/** signal that the solution has been found */
	virtual void CloseStep(void) { };

	/** solution for the current step failed. Restore system to its
	 * state at the beginning of the current time step. */
	virtual void Reset(void) { };
	/*@}*/

	/** returns true if the internal force has been changed since
	 * the last time step */
	virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/** output current configuration */
	virtual void WriteOutput(ostream& out) const;

	/** append connectivities generated by the controller. By default, 
	 * does not append any connectivities to the list */
	virtual void Connectivities(AutoArrayT<const iArray2DT*>& connects) const;

	/** append equation number sets generated by the controller */
	virtual void Equations(AutoArrayT<const iArray2DT*>& equations) const;
	
protected:

	/** read nodes from stream.
	 * \param in input stream listing the node ids
	 * \param id_list returns with the set id's of the nodes
	 * \param nodes returns with the nodes in the set id's */
	void ReadNodes(ifstreamT& in, ArrayT<StringT>& id_list, iArrayT& nodes) const;
	
protected:

	/* nodal data */
	NodeManagerT& fNodeManager;
	
	/* boundary conditions cards - return value */
	ArrayT<KBC_CardT> fKBC_Cards;  	
};

/* boundary condition cards generated by the controller */
inline const ArrayT<KBC_CardT>& KBC_ControllerT::KBC_Cards(void) const
{
	return fKBC_Cards;
}

inline void KBC_ControllerT::Update(const dArrayT& update)
{
#pragma unused(update)
}

inline void KBC_ControllerT::Connectivities(AutoArrayT<const iArray2DT*>& connects) const
{
#pragma unused(connects)
}

inline void KBC_ControllerT::Equations(AutoArrayT<const iArray2DT*>& equations) const
{
#pragma unused(equations)
}

inline void KBC_ControllerT::InitialCondition(void) { }
inline void KBC_ControllerT::Initialize(ifstreamT& in) 
{
#pragma unused(in)
}

} // namespace Tahoe 
#endif /* _KBC_CONTROLLER_T_H_ */
