/* $Id: DiffusionElementT.h,v 1.7 2002-11-14 17:05:51 paklein Exp $ */
/* created: paklein (10/02/1999) */
#ifndef _DIFFUSE_T_H_
#define _DIFFUSE_T_H_

/* base class */
#include "ContinuumElementT.h"

/* direct members */
#include "dArray2DT.h"
#include "LocalArrayT.h"
#include "dSymMatrixT.h"

#include "ios_fwd_decl.h"

namespace Tahoe {

/* forward declarations */
class ShapeFunctionT;
class DiffusionMaterialT;
class DiffusionMatSupportT;
class StringT;

/** linear diffusion element */
class DiffusionElementT: public ContinuumElementT
{
public:
	
	/** list/index of nodal outputs */
	enum OutputCodeT {iNodalCoord = 0,  /**< (reference) nodal coordinates */
                       iNodalDisp = 1,  /**< nodal "displacements" */
                    iMaterialData = 2}; /**< material model output */

	/** constructor */
	DiffusionElementT(const ElementSupportT& support, const FieldT& field);

	/** destructor */
	~DiffusionElementT(void);
	
	/** data initialization */
	virtual void Initialize(void);

	/** TEMPORARY. Need this extra call here to set the source for the iteration number
	 * in SmallStrainT::fSSMatSupport. The solvers are not constructed when the material
	 * support is initialized */
	virtual void InitialCondition(void);

	/** compute nodal force */
	virtual void AddNodalForce(const FieldT& field, int node, dArrayT& force);

	/** returns the stored energy */
	virtual double InternalEnergy(void);

	/** compute specified output parameter and send for smoothing */
	virtual void SendOutput(int kincode);

protected:

	/** \name print element group data */
	/*@{*/
	virtual void PrintControlData(ostream& out) const;
	virtual void EchoOutputCodes(ifstreamT& in, ostream& out);
	/*@}*/

	/** initialization functions */
	/*@{*/
	virtual void SetLocalArrays(void);
	virtual void SetShape(void);
	/*@}*/

	/** construct the effective mass matrix */
	virtual void LHSDriver(void);

	/** form the residual force vector */
	virtual void RHSDriver(void);

	/** set the \e B matrix at the specified integration point */
	void B(int ip, dMatrixT& B_matrix) const;

	/** increment current element */
	virtual bool NextElement(void);	

	/** form the element stiffness matrix */
	virtual void FormStiffness(double constK);

	/** calculate the internal force contribution ("-k*d") */
	virtual void FormKd(double constK);

	/** construct a new material support and return a pointer. Recipient is responsible for
	 * for freeing the pointer.
	 * \param p an existing MaterialSupportT to be initialized. If NULL, allocate
	 *        a new MaterialSupportT and initialize it. */
	virtual MaterialSupportT* NewMaterialSupport(MaterialSupportT* p = NULL) const;

	/** return a pointer to a new material list. Recipient is responsible for
	 * for freeing the pointer. */
	virtual MaterialListT* NewMaterialList(int size);

	/** driver for calculating output values */
	virtual void ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
	                           const iArrayT& e_codes, dArray2DT& e_values);

private:

	/** \name construct output labels array */
	/*@{*/
	virtual void SetNodalOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
		iArrayT& counts) const;
	virtual void SetElementOutputCodes(IOBaseT::OutputModeT mode, const iArrayT& flags,
		iArrayT& counts) const;
	virtual void GenerateOutputLabels(const iArrayT& n_counts,
		ArrayT<StringT>& n_labels, const iArrayT& e_counts, ArrayT<StringT>& e_labels) const;
	/*@}*/

protected:

	/** run time */
	DiffusionMaterialT* fCurrMaterial;

	/** "temperature rate" with local ordering */
	LocalArrayT fLocVel;
	
	/** \name work space */
	/*@{*/
	dMatrixT fD; /**< constitutive matrix          */
	dMatrixT fB; /**< "strain-displacement" matrix */
	dArrayT  fq; /**< heat flow = k_ij T,j         */
	/*@}*/

	/** field gradients over the element. The gradients are only computed
	 * an integration point at a time. */
  	ArrayT<dArrayT> fGradient_list;

	/** parameters */
	static const int NumOutputCodes;

private:

  	/** the material support used to construct materials lists. This pointer
  	 * is only set the first time DiffusionElementT::NewMaterialList is called. */
	DiffusionMatSupportT* fDiffusionMatSupport;
};

} // namespace Tahoe 
#endif /* _DIFFUSE_T_H_ */
