/* $Id: ElementBaseT.h,v 1.5 2001-10-11 00:50:58 paklein Exp $ */
/* created: paklein (05/24/1996) */

#ifndef _ELEMENTBASE_T_H_
#define _ELEMENTBASE_T_H_

#include "GlobalT.h"

/* base class */
#include "iConsoleObjectT.h"

/* direct members */
#include "iArray2DT.h"
#include "ElementMatrixT.h"
#include "dMatrixT.h"
#include "ElementCardT.h"
#include "dArrayT.h"
#include "AutoArrayT.h"
#include "IOBaseT.h"

/* forward declarations */
#include "ios_fwd_decl.h"
class ifstreamT;
class FEManagerT;
class NodeManagerT;
class LocalArrayT;
class LoadTime;
class eControllerT;
template <class TYPE> class RaggedArray2DT;
class iAutoArrayT;
class dArray2DT;
class StringT;

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
class ElementBaseT: public iConsoleObjectT
{
public:

	/** constructor */
	ElementBaseT(FEManagerT& fe_manager);

	/** destructor */
	virtual ~ElementBaseT(void);

	/** class initialization. Among other things, element work space
	 * is allocated and connectivities are read. */
	virtual void Initialize(void);

	/** return a const reference to the top-level manager */
	const FEManagerT& FEManager(void) const;

	/** return a const reference to the run state flag */
	const GlobalT::StateT& RunState(void) const;

	/** return the number of spatial dimensions */
	int NumSD(void) const;

	/** return the number of degrees of freedom per node */
	int NumDOF(void) const;

	/** set the time integration controller */
	virtual void SetController(eControllerT* controller);

	/** re-initialize. signal to element group that the global
	 * equations numbers are going to be reset so that the group
	 * has the opportunity to reconnect and should reinitialize
	 * an dependencies on global equation numbers obtained from the
	 * NodeManagerT */
	virtual void Reinitialize(void);

	/** form of tangent matrix, symmetric by default */
	virtual GlobalT::SystemTypeT TangentType(void) const = 0;
		
	/** call to trigger calculation and assembly of the tangent stiffness */
	void FormLHS(void);
	
	/** call to trigger calculation and assembly of the residual force */
	void FormRHS(void);
	
	/** accumulate the residual force on the specified node
	 * \param node test node
	 * \param force array into which to assemble to the residual force */
	virtual void AddNodalForce(int node, dArrayT& force) = 0;

	/** initialize current time increment */
	virtual void InitStep(void);

	/** close current time increment. Called if the integration over the
	 * current time increment was successful. */
	virtual void CloseStep(void);

	/** restore the element group to its state at the beginning of the
	 * current time step. Called if the integration over the
	 * current time increment was unsuccessful. */
	virtual void ResetStep(void); 

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
	
	/** register element for output. An interface to indicate the element group
	 * must create an OutputSetT and register it with FEManagerT::RegisterOutput
	 * to obtain an output ID that is used to write data to the current
	 * output destination. */
	virtual void RegisterOutput(void) = 0;

	/** write element output. An interface to indicate the element group
	 * gather nodal and element data and send it for output with
	 * FEManagerT::WriteOutput */
	virtual void WriteOutput(IOBaseT::OutputModeT mode) = 0;

	/** compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode) = 0;
	
	/** collecting element connectivities. The element group should collect
	 * the connectivities defining the geometry of the elements and \em append
	 * them to the AutoArrayT that is passed in. */
	virtual void ConnectsX(AutoArrayT<const iArray2DT*>& connects) const;

	/** collecting element connectivities. The element group should collect
	 * the connectivities defining the field variables over the elements and 
	 * \em append them to the AutoArrayT that is passed in. */
	virtual void ConnectsU(AutoArrayT<const iArray2DT*>& connects_1,
	             AutoArrayT<const RaggedArray2DT<int>*>& connects_2) const;
	
	/** return a pointer to the specified LoadTime function */
	LoadTime* GetLTfPtr(int num) const;
	
	/** prepare for a sequence of time steps */
	virtual void InitialCondition(void);

	/** write restart data to the output stream. Should be paired with
	 * the corresponding ElementBaseT::ReadRestart implementation. */
	virtual void WriteRestart(ostream& out) const;

	/** read restart data to the output stream. Should be paired with
	 * the corresponding ElementBaseT::WriteRestart implementation. */
	virtual void ReadRestart(istream& in);

	/* element card data */
	int NumElements(void) const;
	int CurrElementNumber(void) const;
	ElementCardT& ElementCard(int card) const;
	ElementCardT& CurrentElement(void) const;

	/** returns 1 if DOF's are interpolants of the nodal values */
	virtual int InterpolantDOFs(void) const;

	/** construct the field.
	 * \param nodes list of nodes at which the field should be constructed
	 * \param DOFs array of the field degrees of freedom for all nodes */
	virtual void NodalDOFs(const iArrayT& nodes, dArray2DT& DOFs) const;

	/** return the block ID for the specified element */
	int ElementBlockID(int element) const;

	/** weight the computational effort of every node.
	 * \param weight array length number of nodes */
	virtual void WeightNodalCost(iArrayT& weight) const;

protected: /* for derived classes only */

	/** block info indexes. Enum's indicate for connectivity block used by
	 * the element group the location of... */
	enum BlockIndexT {kID = 0, /**< identification for the block */
	            kStartNum = 1, /**< first element number within the group */
	            kBlockDim = 2, /**< number of elements in the block */
	            kBlockMat = 3, /**< material number used for the block */
	       kBlockDataSize = 4  /**< number of items in the block */ }; 

	/* get local element data, X for geometry, U for
	 * field variables */
	const LocalArrayT& SetLocalX(LocalArrayT& localarray); // the geometry
	const LocalArrayT& SetLocalU(LocalArrayT& localarray); // the degrees of freedom

		 	
	/* called by FormRHS and FormLHS */
	virtual void LHSDriver(void) = 0;
	virtual void RHSDriver(void) = 0;

	/* assembling the left and right hand sides */
	void AssembleRHS(void) const;
	void AssembleLHS(void) const;
	
	/* element loop operations */
	void Top(void);
	virtual bool NextElement(void);

	/* print element group data */
	virtual void PrintControlData(ostream& out) const;
	
	/* element data */
	virtual void EchoConnectivityData(ifstreamT& in, ostream& out);
	virtual void ReadConnectivity(ifstreamT& in, ostream& out);
	virtual void WriteConnectivity(ostream& out) const;

	/* generate connectivities with local numbering -
     * returns the number of nodes used by the element group */
	int MakeLocalConnects(iArray2DT& localconnects);
	void NodesUsed(ArrayT<int>& nodes_used) const;

	/* return pointer to block data given the ID */
	const int* BlockData(int block_ID) const;

	/* write all current element information to the stream */
	virtual void CurrElementInfo(ostream& out) const;

	/** (re-)set element cards array */
	void SetElementCards(void);

private:

	/* I/O formats */
	void ReadConnectivity_ASCII(ifstreamT& in, ostream& out, int num_blocks);
	void ReadConnectivity_TahoeII(ifstreamT& in, ostream& out, int num_blocks);
	void ReadConnectivity_ExodusII(ifstreamT& in, ostream& out, int num_blocks);
	void WriteConnectivity_ASCII(ostream& out) const;

	/** return the default number of element nodes. This function is needed
	 * because ExodusII databases (see ExodusT) do not store ANY information about
	 * empty element groups, which causes trouble for parallel execution
	 * when a partition contains no element from a group. */
	virtual int DefaultNumElemNodes(void) const;
	
protected:

	FEManagerT&   fFEManager;
	NodeManagerT* fNodes;

	/* element controller */
	eControllerT* fController;

	/* derived data */
	int	fNumSD;
	int	fNumDOF;
	int fNumElemNodes;	
	int	fNumElemEqnos;
	int	fAnalysisCode;
	
	/* element-by-element info */
	int	fNumElements;
	AutoArrayT<ElementCardT> fElementCards;
	
	/* grouped element arrays */
	iArray2DT fConnectivities;		
	iArray2DT fEqnos;			
	
	/* element tangent matrix and force vector */								
	ElementMatrixT fLHS;
	dArrayT        fRHS;
	
	/* data for multiple connectivity blocks */
	iArray2DT fBlockData; //[n_blocks]: [ID] [1st group element] [size] [material]
};

/* inline functions */

/* up */
inline const FEManagerT& ElementBaseT::FEManager(void) const { return fFEManager; }
inline int ElementBaseT::NumSD(void) const { return fNumSD; }
inline int ElementBaseT::NumDOF(void) const { return fNumDOF; }

/* currElement operations */
inline void ElementBaseT::Top(void) { fElementCards.Top(); }
inline bool ElementBaseT::NextElement(void) { return fElementCards.Next(); }

/* element card */
inline int ElementBaseT::NumElements(void) const { return fNumElements; }
inline int ElementBaseT::CurrElementNumber(void) const { return fElementCards.Position(); }
inline ElementCardT& ElementBaseT::CurrentElement(void) const { return fElementCards.Current(); }
inline ElementCardT& ElementBaseT::ElementCard(int card) const { return fElementCards[card]; }

/* called by FormRHS and FormLHS */
inline void ElementBaseT::LHSDriver(void) { }
inline void ElementBaseT::RHSDriver(void) { }

#endif /* _ELEMENTBASE_T_H_ */
