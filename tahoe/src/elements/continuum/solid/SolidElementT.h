/* $Id: SolidElementT.h,v 1.21 2003-01-29 07:34:34 paklein Exp $ */
#ifndef _ELASTIC_T_H_
#define _ELASTIC_T_H_

/* base class */
#include "ContinuumElementT.h"

/* direct members */
#include "dArray2DT.h"
#include "LocalArrayT.h"
#include "dSymMatrixT.h"

namespace Tahoe {

/* forward declarations */
class ShapeFunctionT;
class SolidMaterialT;
class SolidMatListT;
class StringT;

/** base class of elements for deformation of solids */
class SolidElementT: public ContinuumElementT
{
public:
	
	/** list/index of nodal outputs */
	enum NodalOutputCodeT {
		iNodalCoord = 0, /**< (reference) coordinates */
 	     iNodalDisp = 1, /**< displacements */
       iNodalStress = 2, /**< extrapolated stresses */
         iPrincipal = 3, /**< extrapolated principal stresses */
     iEnergyDensity = 4, /**< extrapolated strain energy density */
        iWaveSpeeds = 5, /**< extrapolated local wave speeds */
      iMaterialData = 6 /**< extrapolated model output */
		};
	
	/** list/index of element outputs */
	enum ElementOutputCodeT {
	      iCentroid = 0, /**< (reference) centroid coordinates */
		      iMass = 1, /**< integrated element mass */
	  iStrainEnergy = 2, /**< integrated strain energy */
	 iKineticEnergy = 3, /**< integrated kinetic energy */
    iLinearMomentum = 4, /**< integrated linear momentum */
          iIPStress = 5, /**< integration point stresses */
    iIPMaterialData = 6  /**< integration point material model output */
      	};

	/** constructor */
	SolidElementT(const ElementSupportT& support, const FieldT& field);

	/** destructor */
	~SolidElementT(void);

	/** \name access to nodal values */
	/*@{*/
	const LocalArrayT& LastDisplacements(void) const;
	const LocalArrayT& Velocities(void) const;
	const LocalArrayT& Accelerations(void) const;

	/** nodal temperatures. Returns NULL if not available */
	const LocalArrayT* Temperatures(void) const { return fLocTemp; };

	/** nodal temperatures from the last time step. Returns NULL if 
	 * not available */
	const LocalArrayT* LastTemperatures(void) const { return fLocTemp_last; };
	/*@}*/
	
	/** initialization. called immediately after constructor */
	virtual void Initialize(void);

	/** form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/* solution calls */
	virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force);
	virtual void AddLinearMomentum(dArrayT& momentum);
	
	/* returns the energy as defined by the derived class types */
	virtual double InternalEnergy(void);

	/* compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode);

	/** strain-displacement options.
	 * \note This really belongs in SmallStrainT; however, will be here for
	 * not to allow input files to be unchanged. */
	enum StrainOptionT {kStandardB = 0, /**< standard strain-displacement matrix */
	                  kMeanDilBbar = 1  /**< mean dilatation for near incompressibility */ };

protected:

	/** stream extraction operator */
	friend istream& operator>>(istream& in, SolidElementT::StrainOptionT& type);

	/** construct list of materials from the input stream */
	virtual void ReadMaterialData(ifstreamT& in);

	/* print element group data */
	virtual void PrintControlData(ostream& out) const;
	virtual void EchoOutputCodes(ifstreamT& in, ostream& out);

	/* initialization functions */
	virtual void SetLocalArrays(void);
	virtual void SetShape(void);

	/** form shape functions and derivatives */
	virtual void SetGlobalShape(void);

	/** construct a new material support and return a pointer. Recipient is responsible for
	 * for freeing the pointer.
	 * \param p an existing MaterialSupportT to be initialized. If NULL, allocate
	 *        a new MaterialSupportT and initialize it. */
	virtual MaterialSupportT* NewMaterialSupport(MaterialSupportT* p = NULL) const;

	/** set the \e B matrix using the given shape function derivatives
	 * Set strain displacement matrix as in Hughes (2.8.20)
	 * \param derivatives of shape function derivatives: [nsd] x [nnd]
	 * \param B destination for B */
	void Set_B(const dArray2DT& derivatives, dMatrixT& B) const;

	/** set B-bar as given by Hughes (4.5.11-16)*/
	void Set_B_bar(const dArray2DT& derivatives, const dArray2DT& mean_gradient, 
		dMatrixT& B);

	/** \name construct the effective mass matrix */
	/*@{*/
	virtual void LHSDriver(GlobalT::SystemTypeT sys_type);
	void ElementLHSDriver(void);
	/*@}*/

	/** \name form the residual force vector */
	/*@{*/
	virtual void RHSDriver(void);
	void ElementRHSDriver(void);
	/*@}*/

	/** increment current element */
	virtual bool NextElement(void);	
	
	/** form the element stiffness matrix
	 * Compute the linearization of the force calculated by SolidElementT::FormKd */
	virtual void FormStiffness(double constK) = 0;

	/** internal force */
	virtual void FormKd(double constK) = 0;
	
	/** return the materials list. \return NULL if fail. */
	const SolidMatListT& StructuralMaterialList(void) const;

	/** driver for calculating output values */
	virtual void ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
	                           const iArrayT& e_codes, dArray2DT& e_values);
	
	/** indicies of elements in the list of material needs */
	enum MaterialNeedsT {kNeedDisp = 0,
	                     kNeedVel  = 1,
	                 KNeedLastDisp = 2};

	/** \name construct output labels array */
	/*@{*/
	virtual void SetNodalOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
		iArrayT& counts) const;
	virtual void SetElementOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
		iArrayT& counts) const;
	virtual void GenerateOutputLabels(const iArrayT& n_counts, ArrayT<StringT>& n_labels, 
		const iArrayT& e_counts, ArrayT<StringT>& e_labels) const;
	/*@}*/

protected:

	/** \name class parameters */
	/*@{*/
	MassTypeT     fMassType;	
	StrainOptionT fStrainDispOpt;
	/*@}*/

	/* propagation direction for wave speeds */
	dArrayT fNormal;
	
	/** \name arrays with local ordering */
	/*@{*/
	LocalArrayT fLocLastDisp; /**< last converged displacements */
	LocalArrayT fLocVel;      /**< nodal velocities */
	LocalArrayT fLocAcc;      /**< nodal accelerations */

	LocalArrayT* fLocTemp;      /**< (optional) nodal temperatures */
	LocalArrayT* fLocTemp_last; /**< (optional) last nodal temperatures */
	/*@}*/

	/* run time */
	SolidMaterialT*  fCurrMaterial;
	ArrayT<ArrayT<bool> > fMaterialNeeds;

	/** incremental heat sources for each element block */
	ArrayT<dArray2DT> fIncrementalHeat;

	/** \name work space */
	/*@{*/
	dArrayT fElementHeat; /**< destination for heat generation. If not length nip, heat not needed */
	dMatrixT fD; /**< constitutive matrix */
	dMatrixT fB; /**< strain-displacement matrix */
	dSymMatrixT fStress; /**< stress vector */	
	/*@}*/

	/* parameters */
	static const int NumNodalOutputCodes;
	static const int NumElementOutputCodes;

	/* flags for stress smoothing */
	bool qUseSimo, qNoExtrap;
};

/* accessors */
inline const LocalArrayT& SolidElementT::LastDisplacements(void) const { return fLocLastDisp; }
inline const LocalArrayT& SolidElementT::Velocities(void) const { return fLocVel; }
inline const LocalArrayT& SolidElementT::Accelerations(void) const { return fLocAcc; }

} // namespace Tahoe 
#endif /* _ELASTIC_T_H_ */
