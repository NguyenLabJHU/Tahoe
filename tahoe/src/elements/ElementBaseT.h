/* $Id: ElementBaseT.h,v 1.35.2.1 2004-01-21 19:09:55 paklein Exp $ */
/* created: paklein (05/24/1996) */
#ifndef _ELEMENTBASE_T_H_
#define _ELEMENTBASE_T_H_

/* base class */
#include "iConsoleObjectT.h"
#include "ParameterInterfaceT.h"

/* direct members */
#include "GlobalT.h"
#include "iArray2DT.h"
#include "ElementMatrixT.h"
#include "dMatrixT.h"
#include "ElementCardT.h"
#include "dArrayT.h"
#include "AutoArrayT.h"
#include "IOBaseT.h"
#include "ElementBlockDataT.h"
#include "ElementSupportT.h"
#ifndef _FRACTURE_INTERFACE_LIBRARY_
#include "FieldT.h"
#endif

#include "ios_fwd_decl.h"

namespace Tahoe {

/* forward declarations */
class ifstreamT;
class LocalArrayT;
class ScheduleT;
class eIntegratorT;
template <class TYPE> class RaggedArray2DT;
class iAutoArrayT;
class dArray2DT;
class StringT;
class SubListT;

#ifndef _FRACTURE_INTERFACE_LIBRARY_
class FieldT;
#endif

/** base class for element types. Initialization of the element classes
 * is accomplished by first setting the time integration controller with
 * ElementBaseT::SetController followed by calling the function 
 * ElementBaseT::Initialize immediately after the constructor. This gives 
 * derived classes the opportunity to override derived class behavior since
 * both functions are virtual. A sequence of time steps begins with a call
 * to ElementBaseT::InitialCondition. A single time step begins with a call to 
 * ElementBaseT::InitStep, followed by one or more calls to ElementBaseT::FormRHS
 * and ElementBaseT::FormLHS (in that order) depending on the solution method.
 * A time step closes with a call to ElementBaseT::CloseStep or ElementBaseT::ResetStep,
 * depending on whether the integration of the step was successful. 
 * ElementBaseT::ResetStep must return the element to its state at the start 
 * of the current time increment. There are number of purely virtual
 * functions that must be implemented by derived classes. */
class ElementBaseT: public iConsoleObjectT, public ParameterInterfaceT
{
public:

	/** element status flags */
	enum StatusT {kOFF = 0,
                   kON = 1,
               kMarked = 2,
               kMarkON = 3,
              kMarkOFF = 4};

	/** constructors */
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	ElementBaseT(const ElementSupportT& support, const FieldT& field);
	ElementBaseT(const ElementSupportT& support);
#else
	ElementBaseT(ElementSupportT& support);
#endif

	/** destructor */
	virtual ~ElementBaseT(void);

	/** \name accessors */
	/*@{*/
	/** the index of this element group within the FEManagerT */
	int ElementGroupNumber(void) const;
	
	/** number of elements */
	int NumElements(void) const { return fElementCards.Length(); };

	/** number of nodes per element. This size is taken from dimensions of the first
	 * entry in the ElementBaseT::fConnectivies array, and therefore isn't valid until
	 * the connectivies have been dimensioned. */
	int NumElementNodes(void) const;

	/** form of tangent matrix, symmetric by default */
	virtual GlobalT::SystemTypeT TangentType(void) const = 0;
	
	/** return the block ID for the specified element */
	const StringT& ElementBlockID(int element) const;

#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/** the source */
	const ElementSupportT& ElementSupport(void) const { return fSupport; };
#else
	/** the fracture_interface modifies ElementSuppportT often enough that it
	    shouldn't be constant in this implementation */
	ElementSupportT& ElementSupport(void) const {return fSupport; };
#endif

#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/** field information */
	const FieldT& Field(void) const;

	/** return a const reference to the run state flag */
	const GlobalT::StateT& RunState(void) const { return fSupport.RunState(); };

	/** the iteration number for the current time increment */
	const int& IterationNumber(void) const;
	
	/** return true if the element contributes to the solution of the
	 * given group. ElementBaseT::InGroup returns true if group is the
	 * same as the group of the FieldT passed in to ElementBaseT::ElementBaseT. */
	virtual bool InGroup(int group) const;
#endif

	/** return a pointer to the specified LoadTime function */
	const ScheduleT* Schedule(int num) const { return fSupport.Schedule(num); };

	/** return the number of spatial dimensions */
	int NumSD(void) const { return fSupport.NumSD(); };

	/** collect the list of element block ID's used by the element group */
	void ElementBlockIDs(ArrayT<StringT>& IDs) const;
	
	/** return the number of degrees of freedom per node */
	int NumDOF(void) const;
	/*@}*/

	/** class initialization. Among other things, element work space
	 * is allocated and connectivities are read. */
	virtual void Initialize(void);

	/** set the active elements.
	 * \param array of status flags for all elements in the group */
	virtual void SetStatus(const ArrayT<StatusT>& status);

	/** compute LHS-side matrix and assemble to solver.
	 * \param sys_type "maximum" LHS matrix type needed by the solver. The GlobalT::SystemTypeT
	 *        enum is ordered by generality. The solver should indicate the most general
	 *        system type that is actually needed. */
	void FormLHS(GlobalT::SystemTypeT sys_type);

	/** compute RHS-side, residual force vector and assemble to solver
	 * \param group equation group to solve */
	void FormRHS(void);
	
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/** accumulate the residual force on the specified node
	 * \param node test node
	 * \param force array into which to assemble to the residual force */
	virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force) = 0;
#endif

	/** initialize current time increment */
	virtual void InitStep(void);

	/** close current time increment. Called if the integration over the
	 * current time increment was successful. */
	virtual void CloseStep(void);

	/** restore the element group to its state at the beginning of the
	 * current time step. Called if the integration over the
	 * current time increment was unsuccessful. */
	virtual GlobalT::RelaxCodeT ResetStep(void); 

	/** element level reconfiguration for the current time increment. This
	 * provides an interface for element-level adaptivity. The nature of
	 * the changes are indicated by the GlobalT::RelaxCodeT return value. */
	virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/** returns the energy as defined by the derived class types */
	virtual double InternalEnergy(void) = 0;

	/** collecting element group equation numbers. This call from the FEManagerT
	 * is a signal to the element group that the equation system is up to date
	 * for the current time increment. The group collects the equation numbers
	 * associated with the nodes in each element and \em appends the group's
	 * equation numbers to the AutoArrayT's that are passed in.
	 * \param eq_1 list for element equations numbers with a \em fixed number of
	 *        equations numbers per element: [nel] x [nen*ndof] 
	 * \param eq_2 list for element equations numbers with a \em variable number of
	 *        equations numbers per element: [nel] x [nen_i*ndof] (i = 0,...,nel) */
	virtual void Equations(AutoArrayT<const iArray2DT*>& eq_1,
		AutoArrayT<const RaggedArray2DT<int>*>& eq_2);

	/** \name writing output */
	/*@{*/	
	/** register element for output. An interface to indicate the element group
	 * must create an OutputSetT and register it with FEManagerT::RegisterOutput
	 * to obtain an output ID that is used to write data to the current
	 * output destination. */
	virtual void RegisterOutput(void) = 0;

	/** write element output. An interface to indicate the element group
	 * gather nodal and element data and send it for output with
	 * FEManagerT::WriteOutput */
	virtual void WriteOutput(void) = 0;

	/** compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode) = 0;
	/*@}*/
	
	/** \name connectivities
	 * Element groups are queries for connectivities for two reasons:
	 * -# ConnectsU are used when reordering equation numbers for minimizing
	 *    the matrix profile for certain kinds of linear solvers
	 * -# ConnectsX are used to compute a load-balanced domain decomposition.
	 *    Note: an experimental version of FEManagerT_mpi::Decompose also used
	 *    ConnectsU to compute the domain decomposition, but this is not usually
	 *    the case. 
	 */
	/*@{*/
	/** collecting element connectivities for geometry. The element group should collect
	 * the connectivities defining the geometry of the elements and \em append
	 * them to the AutoArrayT that is passed in. */
	virtual void ConnectsX(AutoArrayT<const iArray2DT*>& connects) const;

	/** collecting element connectivities for the field. The element group should collect
	 * the connectivities defining the field variables over the elements and 
	 * \em append them to the AutoArrayT that is passed in. */
	virtual void ConnectsU(AutoArrayT<const iArray2DT*>& connects_1,
	             AutoArrayT<const RaggedArray2DT<int>*>& connects_2) const;
	/*@}*/
		
	/** prepare for a sequence of time steps */
	virtual void InitialCondition(void);

#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/** \name restart functions */
	/*@{*/
	/** write restart data to the output stream. Should be paired with
	 * the corresponding ElementBaseT::ReadRestart implementation. */
	virtual void WriteRestart(ostream& out) const;

	/** read restart data to the output stream. Should be paired with
	 * the corresponding ElementBaseT::WriteRestart implementation. */
	virtual void ReadRestart(istream& in);
	/*@}*/
#else
	virtual void WriteRestart(double* outgoingData) const;

	virtual void ReadRestart(double* incomingData);
#endif

	/** \name element card data */
	/*@{*/
	/** read/write information about a particular element */
	ElementCardT& ElementCard(int card);

	/** read-only information about a particular element */
	const ElementCardT& ElementCard(int card) const;

	/** index of the "current" element */
	int CurrElementNumber(void) const;

	/** reference "current" element */
	ElementCardT& CurrentElement(void);

	/** const reference "current" element */
	const ElementCardT& CurrentElement(void) const;
	/*@}*/

	/** returns 1 if DOF's are interpolants of the nodal values */
	virtual int InterpolantDOFs(void) const;

	/** construct the field.
	 * \param nodes list of nodes at which the field should be constructed
	 * \param DOFs array of the field degrees of freedom for all nodes */
	virtual void NodalDOFs(const iArrayT& nodes, dArray2DT& DOFs) const;

	/** weight the computational effort of every node.
	 * \param weight array length number of nodes */
	virtual void WeightNodalCost(iArrayT& weight) const;

	/** array of nodes used by the element group */
	void NodesUsed(ArrayT<int>& nodes_used) const;

	/** contribution to the nodal residual forces. Return the contribution of this element
	 * group to the residual for the given solver group. 
	 * \note ElementBaseT::InternalForce is not implemented and throws ExceptionT::kGeneralFail. 
	 *       Subclasses need to implemented this method if it is required. This may turn out to
	 *       be a more general approach for collecting the total residual for the given solver
	 *       group and could replace the current approach implemented through ElementBaseT::FormRHS. */
	virtual const dArray2DT& InternalForce(int group);

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** describe the parameters needed by the interface */
	virtual void DefineParameters(ParameterListT& list) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& list_name) const;
	/*@}*/

protected: /* for derived classes only */

	/** map the element numbers from block to group numbering */
	void BlockToGroupElementNumbers(iArrayT& elems, const StringT& block_ID) const;

	/** solver group */
	int Group(void) const;

	/** get local element data, X for geometry, U for
	 * field variables */
	/*@{*/
	/** nodes defining the geometry. Collect node in local ordering
	 * using the connectivities from the current element. */
	const LocalArrayT& SetLocalX(LocalArrayT& localarray);

	/** nodes defining the field. Collect node in local ordering
	 * using the connectivities from the current element. */
	const LocalArrayT& SetLocalU(LocalArrayT& localarray);
	/*@}*/

	/** \name drivers called by ElementBaseT::FormRHS and ElementBaseT::FormLHS */
	/*@{*/
	/** form group contribution to the stiffness matrix */
	virtual void LHSDriver(GlobalT::SystemTypeT sys_type) = 0;

	/** form group contribution to the residual */
	virtual void RHSDriver(void) = 0;
	/*@}*/

	/** \name assembling functions
	 * Assemble into the left and right hand sides using information from the
	 * "current" element. This assumes you are using ElementBaseT::Top and
	 * ElementBaseT::NextElement to traverse the ElementCardT's in
	 * ElementBaseT::fElementCards. */
	/*@{*/
	/** assemble values in ElementBaseT::fRHS using the equation number for the
	 * "current" element. */
	void AssembleRHS(void) const;
	void AssembleLHS(void) const;
	/*@}*/
	
	/** \name element loop operations */
	/*@{*/
	/** reset loop */
	void Top(void);
	
	/** advance to next element. \return true if there is another element, 
	 * false otherwise */ 
	virtual bool NextElement(void);
	/*@}*/

#ifndef _FRACTURE_INTERFACE_LIBRARY_
	/* print element group data */
	virtual void PrintControlData(ostream& out) const;
	
	/** echo element connectivity data. Calls ElementBaseT::ReadConnectivity
	 * to read the data and ElementBaseT::WriteConnectivity to write it. */
	virtual void EchoConnectivityData(ifstreamT& in, ostream& out);

	/** default implementation of reading element connectivities. This
	 * implementation read the connectivity data for the element group
	 * using the ModelManagerT interface. This call configures the list
	 * of pointers to the group connectivities in ElementBaseT::fConnectivities,
	 * set the connectivity block information in ElementBaseT::fBlockData,
	 * allocates space for the element equation numbers in ElementBaseT::fEqnos,
	 * and sets the element card array ElementBaseT::fElementCards with a
	 * call to ElementBaseT::SetElementCards. */
	virtual void ReadConnectivity(ifstreamT& in, ostream& out);

	/** write connectivity data to the output stream. If the verbose output flag
	 * is set, determined from FEManagerT::PrintInput, this function writes the
	 * connectivity data to the output stream in text format. */
	virtual void WriteConnectivity(ostream& out) const;
#else
	/* For SIERRA, we don't need ifstreamT to exist */
	virtual void EchoConnectivityData(void);
	
	virtual void ReadConnectivity(void);
#endif

	/** return pointer to block data given the ID */
	const ElementBlockDataT& BlockData(const StringT& ID) const;

	/** write all current element information to the stream */
	virtual void CurrElementInfo(ostream& out) const;

	/** (re-)set element cards array */
	void SetElementCards(void);

private:

	/** return the default number of element nodes. This function is needed
	 * because ExodusII databases (see ExodusT) do not store ANY information about
	 * empty element groups, which causes trouble for parallel execution
	 * when a partition contains no element from a group. */
	virtual int DefaultNumElemNodes(void) const;
	
protected:

	/** time integrator */
	const eIntegratorT* fIntegrator;

	/** element-by-element info */
	AutoArrayT<ElementCardT> fElementCards;
	
	/** \name grouped element arrays */
	/*@{*/
	ArrayT<const iArray2DT*> fConnectivities;		
	ArrayT<iArray2DT> fEqnos;			
	/*@}*/
	
	/** \name element tangent matrix and force vector */								
	/*@{*/
	ElementMatrixT fLHS;
	dArrayT        fRHS;
	/*@}*/
	
	/** data for multiple connectivity blocks. Each row contains the
	 * information for a block of connectivities. The content of each
	 * row is set by ElementBaseT::BlockIndexT. */
	ArrayT<ElementBlockDataT> fBlockData;

private:

	/** \name sources and parameters
	 * Available to sub-classes through access methods */
	/*@{*/
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	const ElementSupportT& fSupport;
#else
	ElementSupportT& fSupport;
#endif

#ifndef _FRACTURE_INTERFACE_LIBRARY_
	const FieldT* fField;
#endif
	/*@}*/
};

/* inline functions */

/* currElement operations */
inline void ElementBaseT::Top(void) { fElementCards.Top(); }
inline bool ElementBaseT::NextElement(void) { return fElementCards.Next(); }

/* element card */
inline int ElementBaseT::CurrElementNumber(void) const { return fElementCards.Position(); }
inline const ElementCardT& ElementBaseT::CurrentElement(void) const { return fElementCards.Current(); }
inline ElementCardT& ElementBaseT::CurrentElement(void) { return fElementCards.Current(); }

inline ElementCardT& ElementBaseT::ElementCard(int card) { return fElementCards[card]; }
inline const ElementCardT& ElementBaseT::ElementCard(int card) const { return fElementCards[card]; }

/* called by FormRHS and FormLHS */
inline void ElementBaseT::LHSDriver(GlobalT::SystemTypeT) { }
inline void ElementBaseT::RHSDriver(void) { }

/* number of nodes per element */
inline int ElementBaseT::NumElementNodes(void) const
{
#if 1
//#if __option(extended_errorcheck)
	if (fConnectivities.Length() > 0 && fConnectivities[0])
		return fConnectivities[0]->MinorDim();
	else
		return 0;
#else
	return fConnectivities[0]->MinorDim();
#endif
}

#ifndef _FRACTURE_INTERFACE_LIBRARY_
/* field information */
inline const FieldT& ElementBaseT::Field(void) const {
#if __option(extended_errorcheck)
	if (!fField) ExceptionT::GeneralFail("ElementBaseT::Field", "field not set");
#endif
	return *fField;
}
#endif

/* solver group */
inline int ElementBaseT::Group(void) const {
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	return Field().Group(); 
#else
	return 0;
#endif
};

/* return the number of degrees of freedom per node */
inline int ElementBaseT::NumDOF(void) const
{ 
#ifndef _FRACTURE_INTERFACE_LIBRARY_
	return Field().NumDOF();
#else
	return fSupport.NumSD();
#endif 
};

} /* namespace Tahoe */

#endif /* _ELEMENTBASE_T_H_ */
