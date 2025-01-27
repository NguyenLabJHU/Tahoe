/* $Id: SCNIMFT.h,v 1.33 2005-09-29 19:19:16 jcmach Exp $ */
#ifndef _SCNIMF_T_H_
#define _SCNIMF_T_H_

/* base class */
#include "ElementBaseT.h"

/* direct members */
#include "VariArrayT.h"
#include "LocalArrayT.h"
#include "MeshFreeNodalShapeFunctionT.h"
#include "MaterialListT.h"
#include "nVariArray2DT.h"
#include "InverseMapT.h"
#include "iArrayT.h"
#include "iArray2DT.h"
#include "dArrayT.h"
#include "dArray2DT.h"
#include "RaggedArray2DT.h"
#include "GeometryBaseT.h"
#include "ScheduleT.h"
#include "LinkedListT.h"

namespace Tahoe {

/** forward declarations */
class iGridManagerT;
class CommManagerT;
class dSPMatrixT; //TEMP
class InverseMapT;
class ifstreamT;
class ofstreamT;
class Traction_CardT;
class CellGeometryT;

/** base class for particle types */
class SCNIMFT: public ElementBaseT
{
public:

	/** constructor */
	SCNIMFT(const ElementSupportT& support, const FieldT& field);
	SCNIMFT(const ElementSupportT& support);

	/** destructor */
	~SCNIMFT(void);
	
	/** form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/** \name initialize/finalize time increment */
	/*@{*/
	virtual void InitStep(void);
	virtual void CloseStep(void);
	virtual GlobalT::RelaxCodeT ResetStep(void); // restore last converged state
	/*@}*/

	/** NOT implemented. Returns an zero force vector */
	virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force);
			
	/** returns the energy as defined by the derived class types */
	virtual double InternalEnergy(void) { return 0.0; };
	
	/** resgiter for writing output. Uses output labels generated by 
	 * small- and finite- strain implementations of this base class.
	 */
	virtual void RegisterOutput(void);

	/* compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode);

	/** trigger reconfiguration */
	virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/** DOF's are not interpolants of the nodal values */
	virtual int InterpolantDOFs(void) const { return 0; };

	/** construct field */
	virtual void NodalDOFs(const iArrayT& nodes, dArray2DT& DOFs) const;

	/** \name restart functions */
	/*@{*/
	/** write restart data to the output stream. Should be paired with
	 * the corresponding ElementBaseT::ReadRestart implementation. */
	virtual void WriteRestart(ostream& out) const;

	/** read restart data to the output stream. Should be paired with
	 * the corresponding ElementBaseT::WriteRestart implementation. */
	virtual void ReadRestart(istream& in);
	/*@}*/

	/** Loop over nodes and compute stiffness matrix */
	virtual void LHSDriver(GlobalT::SystemTypeT sys_type) = 0;
	
	/** Loop over nodes and compute internal force */
	virtual void RHSDriver(void);
	
	/** Generate local equation numbers */
	virtual void Equations(AutoArrayT<const iArray2DT*>& eq_1,
						AutoArrayT<const RaggedArray2DT<int>*>& eq_2);

	/** \name communication routine for MFLagMultT
	/*@{*/
	/** nodes used in global numbering */
	const iArrayT& NodesUsed(void) const { return fNodes; };
	
	/** Translate global node numbers to local ones,
	 * returns 0 if unsucessful, i.e. nodes not contained in fNodes */
	int GlobalToLocalNumbering(ArrayT<int>& nodes) const;
	
	/** Translate global node numbers to local ones */
	int GlobalToLocalNumbering(RaggedArray2DT<int>& nodes);

	/** Return interpolated displacement field at selected nodes */
	void InterpolatedFieldAtNodes(const ArrayT<int>& nodes, dArray2DT& fieldAtNodes) const;

	/** Return the data structure holding the supports of the localNodes and their window function values */
	void NodalSupportAndPhi(const iArrayT& localNodes, RaggedArray2DT<int>& support, 
		RaggedArray2DT<double>& phi) const;
	
	int SupportSize(int localNode) const;
	/*@}*/
	
	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** return the description of the given inline subordinate parameter list */
	virtual void DefineInlineSub(const StringT& name, ParameterListT::ListOrderT& order, 
		SubListT& sub_lists) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& name) const;
	
	virtual void TakeParameterList(const ParameterListT& list);
	
	void TakeNaturalBC(const ParameterListT& list);
	/*@}*/


protected: /* for derived classes only */

	/** list/index of output values */
	enum SCNIOutputCodeT {
		kCoordinates = 0,
		kDisplacement = 1,
		kMass = 2,
		kStrain = 3,
		kStress = 4,
		kMaterialOutput = 5
	};

	/** return number of values for each output variable */
	virtual void SetOutputCount(const iArrayT& flags, iArrayT& counts) const;

	/** echo element connectivity data. Reads parameters that define
	 * which nodes belong to this ParticleT group. */
	virtual void DefineElements(const ArrayT<StringT>& block_ID, const ArrayT<int>& mat_index);
	
	virtual void CollectMaterialInfo(const ParameterListT& all_params, ParameterListT& mat_params) const = 0;
	virtual MaterialListT* NewMaterialList(const StringT& name, int size) = 0;
	
	/** generate labels for output data */
	virtual void GenerateOutputLabels(ArrayT<StringT>& labels);

	/** return true if connectivities are changing */
	virtual bool ChangingGeometry(void) const;

	/** assemble particle mass matrix into LHS of global equation system */
	virtual void AssembleParticleMass(const double rho);

protected:

	/** pointer to list parameters needed to construct shape functions */
	const ParameterListT* fMeshfreeParameters;

	/** connectivities used to define the output set. */
	iArray2DT fPointConnectivities;

	/** \name cached RHS workspace */
	/*@{*/
	dArray2DT fForce;
	nVariArray2DT<double> fForce_man;
	/*@}*/

	/** indices of nodes */
	iArrayT fNodes;

	/** inverse map of nodes */
	InverseMapT fNodes_inv;

	/** coordinates of nodes */	
	dArray2DT fNodalCoordinates;

	/** support class for nodal integration */
	CellGeometryT* fCellGeometry;
	
	int fNumIP;

	/** Volume associated with each node -- integration weight for nodal integration */
	dArrayT fCellVolumes;
	
	/** The centroids of the nodal cells used to compute the mass matrix for the axisymmetric element. */
	dArray2DT fCellCentroids;

	/** list of materials */
	MaterialListT* fMaterialList;

	/** These are the actual data structures used for force and stiffness computations */
	RaggedArray2DT<int> nodalCellSupports;
	RaggedArray2DT<dArrayT> bVectorArray;
	RaggedArray2DT<dMatrixT> bprimeVectorArray;
	dArray2DT Ymatrices;
	
	/** workspace for nodal shape functions */
	RaggedArray2DT<double> fNodalPhi, fBoundaryPhi;
	RaggedArray2DT<int> fNodalSupports, fBoundarySupports;
	  	
	/* body force vector */
	const ScheduleT* fBodySchedule; /**< body force schedule */
	dArrayT fBody; /**< body force vector */
	
	/** shape functions */
	MeshFreeNodalShapeFunctionT* fNodalShapes;
	
	/** underlying Element connectivities. Needed only for MLS stuff right now */
	ArrayT<const iArray2DT*> fElementConnectivities;

	/** equation numbers */
	RaggedArray2DT<int> fEqnos;

	/* traction data */
	dArray2DT fTractionVectors;
	iArrayT fTractionBoundaryCondition;
	
	/** normal vectors around the body boundary -- created by geometry support classes */
	dArray2DT fBoundaryFacetNormals;
	
	/** true if the formulation is Axisymmetric */
	bool qIsAxisymmetric;
	
	/** 1/R information for axisymmetric elements */
	RaggedArray2DT<double> circumferential_B;

	/** output flags */
	iArrayT fOutputFlags;
};

} /* namespace Tahoe */

#endif /* _SCNIMF_T_H_ */


