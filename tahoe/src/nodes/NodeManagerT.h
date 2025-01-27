/* $Id: NodeManagerT.h,v 1.32 2008-05-26 19:04:08 bcyansfn Exp $ */
/* created: paklein (05/23/1996) */
#ifndef _NODEMANAGER_T_H_
#define _NODEMANAGER_T_H_

/* base classes */
#include "iConsoleObjectT.h"
#include "XDOF_ManagerT.h"
#include "GroupAverageT.h"
#include "ParameterInterfaceT.h"

/* direct members */
#include "dArray2DT.h"
#include "iArray2DT.h"
#include "iArrayT.h"
#include "dArrayT.h"
#include "pArrayT.h"
#include "IOBaseT.h"
#include "GlobalT.h"
#include "IntegratorT.h"
#include "nVariArray2DT.h"
#include "FieldSupportT.h"

#ifdef __DEVELOPMENT__
#include "DevelopmentElementsConfig.h"
#ifdef DEM_COUPLING_DEV
#include "FBC_CardT.h"
#endif
#endif

namespace Tahoe {

/* forward declarations */
class FEManagerT;
class CommManagerT;
class ScheduleT;
class LocalArrayT;
class nIntegratorT;
template <class TYPE> class AutoArrayT;
template <class TYPE> class RaggedArray2DT;
class ExodusT;
class ifstreamT;
class ofstreamT;
class iAutoArrayT;
class StringT;
class FieldT;
class FBC_ControllerT;
class KBC_ControllerT;

/** base class for nodes */
class NodeManagerT: 
	public iConsoleObjectT, 
	public XDOF_ManagerT, 
	public GroupAverageT,
	public ParameterInterfaceT
{
public:

	/** constructor */
	NodeManagerT(FEManagerT& fe_manager, CommManagerT& comm_manager);
	
	/** destructor */
	virtual ~NodeManagerT(void);

	/** \name basic MP info */
	/*@{*/
	int Rank(void) const;
	int Size(void) const;
	/*@}*/

	/** \name accessors */
	/*@{*/
	/** const reference to the FEManagerT */
	const FEManagerT& FEManager(void) const;

	/** total number of equations in the given solver group */
	int NumEquations(int group) const;

	/** number of unknowns per node in the given solver group */
	int NumDOF(int group) const;
	int NumNodes(void) const;
	int NumSD(void) const;
	
	/** the total number of fields */
	int NumFields(void) const { return fFields.Length(); };
	
	/** the number of fields in the given group */
	int NumFields(int group) const;

	/** return a const pointer to the field with the specified name. returns NULL
	 * if a field with the given name is not found. */
	const FieldT* Field(const char* name) const;

	/** return a non-const pointer to the field with the specified name. returns NULL
	 * if a field with the given name is not found. */
	FieldT* Field(const char* name);

	/** symmetry/structure of the stiffness matrix for the given group */
	virtual GlobalT::SystemTypeT TangentType(int group) const;

	/** collect fields with the given group ID */
	void CollectFields(int group, ArrayT<FieldT*>& fields) const;

	/** return any field connectivities generated by the node manager. Some
	 * FBC_ControllerT and KBC_ControllerT do generate connectivities */
	virtual void ConnectsU(int group, 
		AutoArrayT<const iArray2DT*>& connects_1,
		AutoArrayT<const RaggedArray2DT<int>*>& connects_2,
		AutoArrayT<const iArray2DT*>& equivalent_nodes) const;

	/** return the implicit-explicit flag for the given group. If the group contains
	 * fields with both implicit and explicit time integrators, the group is considered
	 * implicit */
	IntegratorT::ImpExpFlagT ImplicitExplicit(int group) const;
	/*@}*/

	/** \name writing output */
	/*@{*/
	/** register data for output */
	virtual void RegisterOutput(void);

	/** send output data for writing */
	virtual void WriteOutput(void);
	/*@}*/

	/** \name global equation numbers */
	/*@{*/
	/** assign equation numbers. (Re-)assign equation numbers to the
	 * degrees of freedom in the given group. For groups with more than
	 * one field, equation numbers are assigned so the degrees of freedom
	 * for a given node are numbered sequentially across the fields. */
	virtual void SetEquationNumbers(int group);

	/** renumber the equations in the given group using the information
	 * in the connectivities */
	void RenumberEquations(int group, const ArrayT<const iArray2DT*>& connects_1,
		const ArrayT<const RaggedArray2DT<int>*>& connects_2);

	/** change the numbering scope of all equations in the group */
	virtual void SetEquationNumberScope(int group, GlobalT::EquationNumberScopeT scope);

	/** equation for the given degree of freedom. Equations > 0 are "active" */
	int EquationNumber(int field, int node, int dof) const;

	/** write the equations numbers for all degrees of freedom to the
	 * output stream in groups */
	virtual void WriteEquationNumbers(int group, ostream& out) const;

	/** return any equations sets generated by the node manager for
	 * the specified group. Some FBC_ControllerT and KBC_ControllerT do 
	 * generate equations sets.
	 * These numbers are used to determine the structure of the global
	 * system of equations. This call occurs after the equation numbers 
	 * have been assigned, meaning that any previously cached sets of
	 * equation numbers are stale and need to be re-determined. */
	virtual void Equations(int group, AutoArrayT<const iArray2DT*>& eq_1,
		AutoArrayT<const RaggedArray2DT<int>*>& eq_2);
	/*@}*/

	/** \name tangent and force contributions */
	/*@{*/
	/** compute LHS-side matrix and assemble to solver.
	 * \param group equation group to solve
	 * \param sys_type "maximum" LHS matrix type needed by the solver. The GlobalT::SystemTypeT
	 *        enum is ordered by generality. The solver should indicate the most general
	 *        system type that is actually needed. */
	virtual void FormLHS(int group, GlobalT::SystemTypeT sys_type);

	/** compute RHS-side, residual force vector and assemble to solver
	 * \param group equation group to solve */
	virtual void FormRHS(int group);

        /* compute only the ghost particle contribution to the residual force vector */
#ifdef DEM_COUPLING_DEV
	virtual void FormRHS(int group, ArrayT<FBC_CardT>& fGhostFBC);
#endif

	/** call to signal end of RHS calculation to allow NodeManagerT to post-process
	 * the total system force */
	void EndRHS(int group);

	/** call to signal end of LHS calculation to allow NodeManagerT to post-process
	 * the total system tangent matrix */
	void EndLHS(int group);
	/*@}*/

	/* returns true if the internal force has been changed since
	 * the last time step */
	virtual GlobalT::RelaxCodeT RelaxSystem(int group);

	/** set the time step */
	void SetTimeStep(double dt);

	/** Set to initial conditions */
	virtual void InitialCondition(void);
	
	/** apply kinematic boundary conditions */
	virtual void InitStep(int group);

	/** update history */
	virtual void CloseStep(int group);

	/** update the active degrees of freedom */
	virtual void Update(int group, const dArrayT& update);
	
	/** update the current configuration. This is called by NodeManagerT::Update
	 * and does not usually need to be called explicitly. */
	void UpdateCurrentCoordinates(void);

	/** copy nodal information. Copy all field information from the source 
	 * nodes to the targets. The current coordinates are updated, but the
	 * initial coordinates are not. These are owned by the ModelManagerT. */
	void CopyNodeToNode(const ArrayT<int>& source, const ArrayT<int>& target);

	/** \name packing up all nodal information */
	/*@{*/
	/** size of the nodal package */
	int PackSize(void) const;
	
	/** copy field information into the array */
	void Pack(int node, dArrayT& values) const;

	/** write information from the array into the fields */
	void Unpack(int node, dArrayT& values);
	/*@}*/

	/** reset fields (and configuration to the last known solution) */
	virtual GlobalT::RelaxCodeT ResetStep(int group);

	/** return the current values of the active degrees of freedom 
	 * \param order order of derivative to collect */
	virtual void GetUnknowns(int group, int order, dArrayT& unknowns) const;

	/** \name restart functions
	 * Read and write restart information for all fields. */
	/*@{*/ 
	virtual void ReadRestart(ifstreamT& in);
	virtual void WriteRestart(ofstreamT& out) const;
	/*@}*/ 

	/** \name nodal coordinates and fields */
	/*@{*/
	/** reference configuration */
	const dArray2DT& InitialCoordinates(void) const;

	/** current configuration */
	const dArray2DT& CurrentCoordinates(void) const;

	/** return the time derivative of the specified field */
	const dArray2DT& Field(int field, int order) const;

	/** register the local coordinate array with its source */
	void RegisterCoordinates(LocalArrayT& array) const;
	
	/** the communications manager */
	CommManagerT& CommManager(void) const;

	/** read/write access to the coordinate update field. Returns NULL if these is no
	 * coordinate update field. */
	dArray2DT* CoordinateUpdate(void);
	/*@}*/

	/* weight the computational effort of every node */
	virtual void WeightNodalCost(iArrayT& weight) const;

	/** \name dynamic transformations */
	/*@{*/
	/** reset the number of nodes. Resizes all coordinate and field arrays to
	 * the new number of nodes. Excess nodes at the tail of all arrays are
	 * discarded. Additional nodes added to all arrays is not initialized. */
	void ResizeNodes(int num_nodes);
	/*@}*/

	/** return a pointer to the specified schedule. The schedule
	 * number is passed to the FEManagerT for resolution. */
	const ScheduleT* Schedule(int num) const;

	/** \name XDOF interface */
	/*@{*/
	/** add element group to list. See XDOF_ManagerT::Register. This
	 * function is just added to catch XDOF groups for parallel
	 * execution. XDOF's have not been verified for parallel. */
	virtual void XDOF_Register(DOFElementT* group, const iArrayT& numDOF);

	/** collection equation numbers for mixed connectivities. See documentation
	 * XDOF_ManagerT.  */
	virtual void XDOF_SetLocalEqnos(int group, const iArrayT& nodes, iArray2DT& eqnos);

	/** collection equation numbers for mixed connectivities. See documentation
	 * XDOF_ManagerT.  */
	virtual void XDOF_SetLocalEqnos(int group, const iArray2DT& nodes, iArray2DT& eqnos) const;

	/** collection equation numbers for mixed connectivities. See documentation
	 * XDOF_ManagerT. */
	virtual void XDOF_SetLocalEqnos(int group, const RaggedArray2DT<int>& nodes, RaggedArray2DT<int>& eqnos) const;
	/*@}*/

	/** \name construct BC controllers */
	/*@{*/
	virtual KBC_ControllerT* NewKBC_Controller(FieldT& field, int code);
	virtual FBC_ControllerT* NewFBC_Controller(int code);
	/*@}*/

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** describe the parameters needed by the interface*/
	virtual void DefineParameters(ParameterListT& list) const;
	
	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& name) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

protected:

	/** \name steps of NodeManagerT::Initialize */
	/*@{*/
	void SetCoordinates(void);
	/*@}*/

	/** simple output function */
	virtual void WriteData(ostream& out, const char* title, const char* name,
		const dArray2DT& data, const iArrayT* rowlabels) const;

	/** \name renumbering equation
	 * methods over the specified group */
	/*@{*/
	virtual void EquationNumbers(int group, AutoArrayT<iArray2DT*>& equationsets);
	virtual void CheckEquationNumbers(int group);
	/*@}*/

private:

	/** write nodal history data. Called by NodeManagerT:WriteOutput */
	virtual void WriteNodalHistory(void);

	/** \name not allowed */
	/*@{*/
	/** copy constructor */
	NodeManagerT(NodeManagerT&);
	
	/** assignment operator */
	const NodeManagerT& operator=(const NodeManagerT&);
	/*@}*/

protected:

	/** host */
	FEManagerT& fFEManager;
	
	/** communication manager */
	CommManagerT& fCommManager;

	/** \name fields */
	/*@{*/
	/** support for fields */
	FieldSupportT fFieldSupport;
	
	/** array of fields */
	ArrayT<FieldT*> fFields;

	/** ID for the field exchange obtained from NodeManagerT::fCommManager */
	iArrayT fMessageID;
	/*@}*/

	/** \name history nodes information */
	/*@{*/
	/** ID's of the history nodes. The node sets are registerered with the
	 * ModelManagerT as connectivities: [nsets]*/
	ArrayT<StringT> fHistoryNodeSetIDs;
	                                     
	/** history node output set information. There is one output set
	 * per field for every history node set: [nfield*nset] x [3]
	 * Each row has:
	 * -# output set ID returned by FEManagerT::RegisterOutput
	 * -# index of the field associated with the output set 
	 * -# index in NodeManagerT::fHistoryNodeSetIDs of
	 *    the "connectivitities" associated with the set. */
	iArray2DT fHistoryOutputID;
	/*@}*/
	
	/** \name section of field update by this processor */
	/*@{*/
	int fCoordUpdateIndex;
	iArrayT fFieldStart;
	iArrayT fFieldEnd;
	/*@}*/
	
private:

	/** \name nodal coordinates */
	/*@{*/
	/** initial coordinate array. Owned by the ModelManagerT. Not initialized 
	 * until NodeManagerT::EchoCoordinates. */
	const dArray2DT* fInitCoords;
	
	/** the field that updates the current coordinates. Pointer is NULL if
	 * there is no update from the initial coordinates to the current coords */
	FieldT* fCoordUpdate;

	/** current coordinates. NULL if the current coordinates are the same
	 * as the initial coordinates. */
	dArray2DT* fCurrentCoords;
	nVariArray2DT<double> fCurrentCoords_man;

	/** true of initial coordinates requested through NodeManagerT::RegisterCoordinates
	 * or through NodeManagerT::CurrentCoordinates.
	 * \note This flag isn't used yet, but could be used to determine if the current
	 *       coordinates need to be updated, which may be expensive if FEManagerT::InterpolantDOFs
	 *       is 0, i.e., for meshfree methods. */
	bool fNeedCurrentCoords;
	/*@}*/

	/** highest precedence relaxation code return by XDOF_ManagerT::ResetTags */
	ArrayT<GlobalT::RelaxCodeT> fXDOFRelaxCodes;
};

/* inlines */

/* return a const pointer to the field with the specified name */
inline FieldT* NodeManagerT::Field(const char* name)
{
	/* const this */
	const NodeManagerT* this_ = (const NodeManagerT*) this;
	const FieldT* field = this_->Field(name);
	return (FieldT*) field;
}

/* reference configuration */
inline const dArray2DT& NodeManagerT::InitialCoordinates(void) const
{
	if (!fInitCoords)
		ExceptionT::GeneralFail("NodeManagerT::InitialCoordinates", "array not set");
	return *fInitCoords;
}

/* current configuration */
inline const dArray2DT& NodeManagerT::CurrentCoordinates(void) const
{
	/* non-const this */
	NodeManagerT* non_const_this = (NodeManagerT*) this;
	non_const_this->fNeedCurrentCoords = true;
	if (fCurrentCoords)
		return *fCurrentCoords;
	else
		return InitialCoordinates();
}

/* accessors */
inline const FEManagerT& NodeManagerT::FEManager(void) const { return fFEManager; }
inline int NodeManagerT::NumNodes(void) const 
{ 
	return InitialCoordinates().MajorDim(); 
}

inline int NodeManagerT::NumSD(void) const 
{ 
	return InitialCoordinates().MinorDim(); 
}

} // namespace Tahoe 
#endif /* _NODEMANAGER_T_H_ */
