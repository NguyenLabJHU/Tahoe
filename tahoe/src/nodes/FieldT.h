/* $Id: FieldT.h,v 1.18.2.1 2004-01-28 01:34:12 paklein Exp $ */
#ifndef _FIELD_T_H_
#define _FIELD_T_H_

/* base class */
#include "BasicFieldT.h"
#include "ParameterInterfaceT.h"

/* direct members */
#include "iArrayT.h"
#include "dArrayT.h"
#include "IC_CardT.h"
#include "KBC_CardT.h"
#include "FBC_CardT.h"
#include "GlobalT.h"
#include "AutoArrayT.h"

namespace Tahoe {

/* forward declarations */
class LocalArrayT;
class IntegratorT;
class nIntegratorT;
class KBC_ControllerT;
class FBC_ControllerT;
template <class TYPE> class RaggedArray2DT;
class ifstreamT;
class ofstreamT;
class FieldSupportT;

/** field with time integration. Includes application of initial and 
 * boundary conditions, and force boundary conditions. */
class FieldT: public BasicFieldT, public ParameterInterfaceT
{
public:

	/** inactive equation number codes */
	enum EquationCodeT {
	          kInit = 0, /**< uninitialized equation number */
		kPrescribed =-1, /**< dof has prescribed value */ 
	      kExternal =-2  /**< node is external */ };

	/** constructor */
	FieldT(const FieldSupportT& field_support);
	
	/** destructor */
	~FieldT(void);

	/** \name initialization */
	/*@{*/
	/** configure the field */
	void Initialize(const StringT& name, int ndof, int order);
	
	/** register the local array with its source */
	void RegisterLocal(LocalArrayT& array) const;

	/** set the group number */
	void SetGroup(int group) { fGroup = group; };

	/** set number of nodes */
	virtual void Dimension(int nnd, bool copy_in);

	/** set all field values to 0.0 */
	virtual void Clear(void);
	/*@}*/
	
	/** \name accessors */
	/*@{*/
	/** reference to the specified derivative of the field of the given
	 * relative time increment. step = 0 is the current step, -1 is the
	 * previous step */ 
	dArray2DT& operator()(int step, int order);

	/** const reference to the specified derivative of the field at given step */ 
	const dArray2DT& operator()(int step, int order) const;

	/** set the group number */
	int Group(void) const { return fGroup; };	
	
	/** \name time integrator */
	/*@{*/
	nIntegratorT& nIntegrator(void);
	const nIntegratorT& nIntegrator(void) const;
	/*@}*/

	/** append connectivities generated by the KBC_ControllerT's and
	 * FBC_ControllerT's. */
	void Connectivities(AutoArrayT<const iArray2DT*>& connects_1,
		AutoArrayT<const RaggedArray2DT<int>*>& connects_2,
		AutoArrayT<const iArray2DT*>& equivalenceQ) const;

	/** return the GlobalT::SystemTypeT for the all the fields in the
	 * specified group */
	GlobalT::SystemTypeT SystemType(void) const;

	/** initial condition cards */
	ArrayT<IC_CardT>& InitialConditions(void) { return fIC; };
	
	/** kinematic boundary condition cards */
	ArrayT<KBC_CardT>& KinematicBC(void) { return fKBC; };

	/** prescribed nodal forces */
	ArrayT<FBC_CardT>& ForceBC(void) { return fFBC; };

	/** special kinematic boundary conditions. Controllers put in this array
	 * are deleted when this field goes out of scope. */
	const ArrayT<KBC_ControllerT*>& KBC_Controllers(void) const { return fKBC_Controllers; };

	/** special force boundary conditions. Controllers put in this array
	 * are deleted when this field goes out of scope. */
	const ArrayT<FBC_ControllerT*>& FBC_Controllers(void) const { return fFBC_Controllers; };
	/*@}*/

	/** \name time integration */
	/*@{*/
	/** beginning of time series */
	void InitialCondition(void);
	
	/** apply predictor to all degrees of freedom */
	void InitStep(void);

	/** \name form tangent and force contributions */
	/*@{*/
	/** compute LHS-side matrix and assemble to solver.
	 * \param support host information
	 * \param sys_type "maximum" LHS matrix type needed by the solver. The GlobalT::SystemTypeT
	 *        enum is ordered by generality. The solver should indicate the most general
	 *        system type that is actually needed. */
	void FormLHS(GlobalT::SystemTypeT sys_type);

	/** compute RHS-side, residual force vector and assemble to solver
	 * \param support host information */
	void FormRHS(void);
	/*@}*/

	/** \name update array.
	 * Updates are applied to the field by calling FieldT::Update. Values can be written
	 * into the update array in three ways:
	 * -# FieldT::AssembleUpdate overwrites values for the equations corresponding to 
	 *    the parameters passed to FieldT::FinalizeEquations 
	 * -# values can be written directly into the update array by accessing it
	 *    with FieldT::Update
	 * Also, field values can be copied from node to node using FieldT::CopyNodeToNode 
	 */
	/*@{*/
	/** read/write access to the update array */
	dArray2DT& Update(void) { return fUpdate; };
	
	/** overwrite the update values in the FieldT::Update array. The update
	 * array corresponds to the parameters passed in to FieldT::FinalizeEquations.
	 * The algorithm properly assembles duplicated numbers in the equations array. */
	void AssembleUpdate(const dArrayT& update);
	
	/** apply the full update to the field */
	void ApplyUpdate(void);

	/** copy nodal information. Copy all field information from the source 
	 * nodes to the targets. Equation are not copied. */
	void CopyNodeToNode(const ArrayT<int>& source, const ArrayT<int>& target);
	/*@}*/
	
	/** check for relaxation */
	GlobalT::RelaxCodeT RelaxSystem(void);

	/** update history */
	void CloseStep(void);

	/** reset displacements (and configuration to the last known solution) */
	GlobalT::RelaxCodeT ResetStep(void);

	/** \name equation numbers
	 * FieldT assumes equation numbers will be assigned by the host. The array can be 
	 * accessed using BasicFieldT::Equations. Configuring the equations requires the
	 * following step:
	 * -# call FieldT::InitEquations to dimension the equations array and initialize
	 *    the values in it.
	 * -# access the equations array using BasicFieldT::Equations and assign equation
	 *    numbers to all active slots
	 * -# call FieldT::FinalizeEquations to reset internal data associated with the
	 *    equations array. */
	/*@{*/
	/** initialize internal equations array. The array is dimensioned for the number
	 * of nodes and dof's. Prescribed equations are marked FieldT::kPrescribed. All 
	 * others are marked FieldT::kInit. Prescribed values are marked first
	 * using the nodally prescribed kinematic boundary conditions
	 * followed by the KBC_ControllerT's. Equation numbers for prescribed
	 * values will be < 0. The array can be accessed using BasicFieldT::Equations.
	 * FieldT assumes equation numbers will be assigned by the host. */
	void InitEquations(void);

	/** reset internal data associated with the equations array. The number of
	 * active equations is continuous but may start with any value. The active
	 * equations are those that correspond to the values passed in by FieldT::AssembleUpdate.
	 * \param eq_start first active equation in the group to which the field belongs
	 * \param num_eq number of active equations in the group to which the field belongs */
	void FinalizeEquations(int eq_start, int num_eq);
	
	/** access to the number of active equations in the group to which the field belongs. 
	 * Returns the value set using FieldT::FinalizeEquations. */
	int NumEquations(void) const { return fNumEquations; };

	/** first active equation number in the group to which the field belongs. Returns 
	 * the value set using FieldT::FinalizeEquations. */
	int EquationStart(void) const { return fEquationStart; };

	/** append the equation sets generated by the field. These include sets
	 * generated by the KBC_ControllerT's and FBC_ControllerT's. This call
	 * signals to the field that FieldT::Equations has been filled with
	 * up to date equation numbers. */
	void EquationSets(AutoArrayT<const iArray2DT*>& eq_1, 
		AutoArrayT<const RaggedArray2DT<int>*>& eq_2);

	/** collect equation numbers.
	 * \param nodes element connectivities: [nel] x [nen]
	 * \param eqnos destination for equation numbers: [nel] x [nen*ndof] */
	void SetLocalEqnos(const iArray2DT& nodes, iArray2DT& eqnos) const;
	
	/** collect equation numbers.
	 * \param nodes element connectivities: [ngr] x [nel_i] x [nen]
	 * \param eqnos destination for equation numbers: [nel_0 + nel_1 + ...] x [nen*ndof] */
	void SetLocalEqnos(ArrayT<const iArray2DT*> nodes, iArray2DT& eqnos) const;

	/** collect equation numbers. Connectivities are passed in a RaggedArray2DT, 
	 * which allows an arbitrary number of nodes per element.
	 * \param nodes element connectivities: [nel] x [nen_i]
	 * \param eqnos destination for equation numbers: [nel] x [nen_i*ndof] */
	void SetLocalEqnos(const RaggedArray2DT<int>& nodes, RaggedArray2DT<int>& eqnos) const;

	/** collect equation numbers */
	void SetLocalEqnos(const ArrayT<int>& tags, iArray2DT& eqnos) const;
	/*@}*/

	/** \name restart functions
	 * The restart functions should read/write any data that overrides the
	 * default values 
	 * \param nodes list of nodes for which field information should be read/written. If
	 *        the pointer is NULL, read/write info for \e all nodes. */
	/*@{*/ 
	void WriteRestart(ofstreamT& out, const ArrayT<int>* nodes) const;
	void ReadRestart(ifstreamT& in, const ArrayT<int>* nodes);
	/*@}*/ 

	/** register results for output */
	void RegisterOutput(void);

	/** write output data */
	void WriteOutput(ostream& out) const;

	/** write field parameters to output stream */
	void WriteParameters(ostream& out) const;
	
	/** \name source terms 
	 * Support multiple sources to the same block ID. The current
	 * implementation assumes the contributors of source terms have
	 * the same time increment as the users of the sources. That is,
	 * the most recently calculated values in the source arrays are
	 * assumed to be up to date. There is no separate accumulation
	 * for sub-steps. 
	 * \note Could add some information about the interval. This
	 *       would allow contributors to decide whether or not
	 *       source terms need to be accumulated or overwritten. */
	/*@{*/
	/** register an array as source. Owner is responsible for keeping the
	 * source array up to date. No assumptions are made on how many times
	 * the same source array can be registered. Arrays registered more
	 * that once will contribute more than once. */
	void RegisterSource(const StringT& ID, const dArray2DT& source) const;
	
	/** element source terms. return a pointer to the source terms for the 
	 * given element block ID, or NULL if no source terms exist for that
	 * block. This accumulates all source contributions to the given
	 * block ID. */
	const dArray2DT* Source(const StringT& ID) const;
	/*@}*/

	/** add the KBC controller to the field. The field takes ownership of the
	 * controller and will take care of de-allocating it */
	void AddKBCController(KBC_ControllerT* controller) { fKBC_Controllers.AppendUnique(controller); };

	/** add the KBC controller to the field. The field takes ownership of the
	 * controller and will take care of de-allocating it */
	void AddFBCController(FBC_ControllerT* controller) { fFBC_Controllers.AppendUnique(controller); };

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** describe the parameters needed by the interface */
	virtual void DefineParameters(ParameterListT& list) const;

	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** return the description of the given inline subordinate parameter list */
	virtual void DefineInlineSub(const StringT& sub, ParameterListT::ListOrderT& order, 
		SubListT& sub_sub_list) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& list_name) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

private:

	/** apply the IC_CardT to the field. Return false if any initial conditions where
	 * defined inactive equations. */
	bool Apply_IC(const IC_CardT& card);

	/** mark global equations with the specified BC */
	void SetBCCode(const KBC_CardT& card);

	/** determine the destinations for the force boundary conditions */
	void SetFBCEquations(void);

	/** return the index for the source of the given ID. Returns -1 if
	 * if no source exists for given ID */
	int SourceIndex(const StringT& ID) const;

private:

	/** support class */
	const FieldSupportT& fFieldSupport;

	/** solution set number */
	int fGroup;

	/** time integrator */
	const IntegratorT* fIntegrator;

	/** nodal interface to time integrator in FieldT::fIntegrator */
	const nIntegratorT* fnIntegrator;
	
	/** field history. BasicFieldT::fField from the previous time step. */
	ArrayT<dArray2DT> fField_last;
	
	/** \name initial and kinematic boundary conditions */
	/*@{*/
	/** initial conditions */
	ArrayT<IC_CardT> fIC;
	  	
	/** kinematic boundary conditions */
	ArrayT<KBC_CardT> fKBC;

	/** special KBC objects */
	AutoArrayT<KBC_ControllerT*> fKBC_Controllers;
	/*@}*/

	/** \name force boundary conditions */
	/*@{*/
	/** nodal forces */
	ArrayT<FBC_CardT> fFBC;

	/** force vector */
	dArrayT fFBCValues;

	/** equations with applied forces */
	iArrayT fFBCEqnos;

	/** special FBC objects */
	AutoArrayT<FBC_ControllerT*> fFBC_Controllers;
	/*@}*/
	
	/** \name update array */
	/*@{*/
	dArray2DT fUpdate;
	int fEquationStart;
	int fNumEquations;
	/*@}*/
	
	/** \name source terms */
	/*@{*/
	/** ID's for the elements blocks in fSource. This array is
	 * a union of the ID's in FieldT::fSourceID. */
	AutoArrayT<StringT> fID;

	/** return values for sources. When FieldT::Source is called, all
	 * contributions to the given block are accumulated and passed back.
	 * Each entry is: [nen] x [nip*nval] */
	AutoArrayT<dArray2DT*> fSourceOutput;
	
	/** ID's for the sources registerered with FieldT::RegisterSource */
	AutoArrayT<StringT> fSourceID;
	
	/** block source terms registerered with FieldT::RegisterSource.
	 * Each entry is: [nen] x [nip*nval] */
	AutoArrayT<const dArray2DT*> fSourceBlocks;
	/*@}*/
};

/* inlines */

/* accessors */ 
inline dArray2DT& FieldT::operator()(int step, int order)
{
	if (step != 0 && step != -1) ExceptionT::OutOfRange("FieldT::operator()");
	if (step == 0)
		return fField[order];
	else
		return fField_last[order];
}

inline const dArray2DT& FieldT::operator()(int step, int order) const
{
	if (step != 0 && step != -1) ExceptionT::OutOfRange("FieldT::operator()");
	if (step == 0)
		return fField[order];
	else
		return fField_last[order];
}

inline void FieldT::SetLocalEqnos(const ArrayT<int>& tags,
	iArray2DT& eqnos) const
{
	eqnos.RowCollect(tags,fEqnos);
}

/* time integrator */
inline const nIntegratorT& FieldT::nIntegrator(void) const {
	if (!fnIntegrator) ExceptionT::GeneralFail("FieldT::nIntegrator");
	return *fnIntegrator;
}

inline nIntegratorT& FieldT::nIntegrator(void) {
	if (!fnIntegrator) ExceptionT::GeneralFail("FieldT::nIntegrator");
	return const_cast<nIntegratorT&>(*fnIntegrator);
}


} /* namespace Tahoe */

#endif /* _FIELD_T_H_ */
