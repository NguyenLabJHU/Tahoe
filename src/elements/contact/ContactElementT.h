/* $Id: ContactElementT.h,v 1.28 2002-11-14 15:43:59 rjones Exp $ */

// DEVELOPMENT

#ifndef _CONTACT_ELEMENT_T_H_
#define _CONTACT_ELEMENT_T_H_

/* base class */
#include "ElementBaseT.h"
#include "DOFElementT.h"

/* direct members */
#include "pArrayT.h"
#include "LocalArrayT.h"
#include "iArrayT.h"
#include "dArrayT.h"
#include "dArray2DT.h"
#include "VariArrayT.h"
#include "nVariArray2DT.h"
#include "nMatrixT.h"
#include "nVariMatrixT.h"
#include "ElementMatrixT.h"
#include "ContactSurfaceT.h"
#include "ContactSearchT.h"


namespace Tahoe {

/* forward declarations */
class XDOF_ManagerT;

/**
ContactElementT is a "super element" of sorts since it is is made up
of smaller entities called ContactSurfaceT's. The connectivities generated
by this element are variable size and can change from time step to time step.
...what else would you like to know?
*/
class ContactElementT: public ElementBaseT, public DOFElementT
{
public:

	/* constructor */
	ContactElementT(const ElementSupportT& support, const FieldT& field, int num_enf_params);

	/* constructor for elements with multipliers */
	ContactElementT(const ElementSupportT& support, const FieldT& field, int num_enf_params, 
		XDOF_ManagerT* xdof_nodes);

	/* destructor */
	virtual ~ContactElementT(void);

	/* form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/* element level reconfiguration for the current solution */
	virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/* initialization after constructor */
	virtual void Initialize(void);

	/* solution calls */
	virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force); //not implemented

	/* Returns the energy as defined by the derived class types */
	virtual double InternalEnergy(void); // not implemented
	
	/* writing output */
	virtual void RegisterOutput(void);
	virtual void WriteOutput(IOBaseT::OutputModeT mode);

	/* compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode);  // not implemented

	/* append element equations numbers to the list */
	virtual void Equations(AutoArrayT<const iArray2DT*>& eq_1,
                AutoArrayT<const RaggedArray2DT<int>*>& eq_2);

	/* append connectivities */
	virtual void ConnectsU(AutoArrayT<const iArray2DT*>& connects_1,
		AutoArrayT<const RaggedArray2DT<int>*>& connects_2) const;
	/* returns no (NULL) geometry connectivies */
	virtual void ConnectsX(AutoArrayT<const iArray2DT*>& connects) const;
	 	
	/* surface specification modes */
	enum SearchParametersT { 	kGapTol = 0,
								kXiTol ,
								kPass,
								kSearchNumParameters};
	int fNumEnfParameters;

	iArrayT fOutputFlags;
	enum OutputFlagsT {kGaps = 0,
			kNormals,
			kStatus,
			kMultipliers,
			kArea,
			kNumOutputFlags};

	/** \name implementation of the DOFElementT interface */
	/*@{*/
	/* returns the array for the DOF tags needed for the current config */
	virtual void SetDOFTags(void);
	virtual iArrayT& DOFTags(int tag_set);

	/** generate nodal connectivities. Since the sequence of setting global equation
     * number is controlled externally, responsibility for calling the element group 
     * to (self-) configure is also left to calls from the outside. otherwise it's tough 
     * to say whether data requested by the group is current. 
     */
	virtual void GenerateElementData(void);

	/** return the contact elements */
  	virtual const iArray2DT& DOFConnects(int tag_set) const;

   	/** restore the DOF values to the last converged solution */
   	virtual void ResetDOF(dArray2DT& DOF, int tag_set) const;
	
   	/** returns 1 if group needs to reconfigure DOF's, else 0 */
   	virtual int Reconfigure(void);

	/** return the solver group number */
	int Group(void) const { return ElementBaseT::Group(); };
	/*@}*/

	inline bool HasMultipliers (void) {return fXDOF_Nodes;}

    enum PassTypeT {kSymmetric = 0,
                    kPrimary,
                    kSecondary,
                    kDeformable,
                    kRigid};

protected:

	/* contact surfaces */
	ArrayT<ContactSurfaceT> fSurfaces; 

	/* search interaction parameters, symmetric matrix */
  	nMatrixT<dArrayT> fSearchParameters ;

	nMatrixT<dArrayT> fEnforcementParameters ;

	/* read element group data */
	void ReadControlData(void);

	/* print element group data */
	virtual void PrintControlData(ostream& out) const;
	
	/* initialization steps */
	virtual void EchoConnectivityData(ifstreamT& in, ostream& out);

	/* returns pass type for surface pair */
    int PassType(int s1,int s2) const;

	/* additional workspace setup */
	virtual void SetWorkspace(void);
	/* workspace data */
	dArrayT n1;
	/* residual */
	dArrayT RHS;
	VariArrayT<double> RHS_man;
	dArrayT tmp_RHS;
	VariArrayT<double> tmp_RHS_man;
	dArrayT xRHS;
    VariArrayT<double> xRHS_man;
	/* stiffness */
	ElementMatrixT LHS; //should be using fLHS
	nVariMatrixT <double> LHS_man;
	ElementMatrixT tmp_LHS; //should be using fLHS
	nVariMatrixT <double> tmp_LHS_man;
	/* shape functions */
	dMatrixT N1;
	nVariMatrixT<double> N1_man;
	dMatrixT N2;
	nVariMatrixT<double> N2_man;
	dArrayT N1n;
	VariArrayT<double> N1n_man;
	dArrayT N2n;
	VariArrayT<double> N2n_man;
	/* integration weights */
	dArrayT weights;
	VariArrayT<double> weights_man;
	/* integration points */
	dArray2DT points; // Maybe should be a pointer and const
	/* equation numbers */
	iArray2DT eqnums1;
	nVariArray2DT<int> eqnums1_man;
	iArray2DT eqnums2;
	nVariArray2DT<int> eqnums2_man;
	iArrayT xconn1;
	VariArrayT<int> xconn1_man;
	iArrayT xconn2;
	VariArrayT<int> xconn2_man;
	iArray2DT xeqnums1;
	nVariArray2DT<int> xeqnums1_man;
	iArray2DT xeqnums2;
	nVariArray2DT<int> xeqnums2_man;
	/* pressure interpolations */
	dMatrixT P1;
    nVariMatrixT<double> P1_man;
    dMatrixT P2;
    nVariMatrixT<double> P2_man;
	ArrayT<double*> P1values;
    VariArrayT<double*> P1values_man;
    ArrayT<double*> P2values;
    VariArrayT<double*> P2values_man;



	/* generate contact element data  */
	bool SetContactConfiguration(void);

	/* update contact element data  */
	bool UpdateContactConfiguration(void);

	/* search pointer */
	ContactSearchT* fContactSearch ;

	/* link surfaces in ConnectsU - for graph */
	iArray2DT fSurfaceLinks;

	/* nodemanager with external DOF's for multipliers */
	XDOF_ManagerT* fXDOF_Nodes;
	int fNumMultipliers;

	
private:
	/* surface specification modes */
	enum SurfaceSpecModeT { kSideSets = 1};
};

} // namespace Tahoe 
#endif /* _CONTACT_ELEMENT_T_H_ */
