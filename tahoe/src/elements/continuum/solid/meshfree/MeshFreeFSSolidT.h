/* $Id: MeshFreeFSSolidT.h,v 1.4.2.1 2002-06-27 18:02:50 cjkimme Exp $ */
/* created: paklein (09/16/1998) */

#ifndef _EFG_FDELASTIC_T_H_
#define _EFG_FDELASTIC_T_H_

/* base classes */
#include "TotalLagrangianT.h"
#include "MeshFreeFractureSupportT.h"

/* direct members */
#include "nVariMatrixT.h"

/** large deformation elasticity with MLS shapefunctions for the
 * field (displacement) representation
 * \note clean up code governing when crack growth algorithm
 * is used, initiation criteria, etc. (PAK 09/28/1999) */

namespace Tahoe {

class MeshFreeFSSolidT: public TotalLagrangianT,
	public MeshFreeFractureSupportT
{
public:

	/* constructor */
	MeshFreeFSSolidT(const ElementSupportT& support, const FieldT& field);
	
	/* data initialization */
	virtual void Initialize(void);

	/* append element equations numbers to the list */
	virtual void Equations(AutoArrayT<const iArray2DT*>& eq_1,
		AutoArrayT<const RaggedArray2DT<int>*>& eq_2);

	/* appends group connectivities to the array */
	virtual void ConnectsU(AutoArrayT<const iArray2DT*>& connects_1,
	AutoArrayT<const RaggedArray2DT<int>*>& connects_2) const;

	/* write output */
	virtual void WriteOutput(IOBaseT::OutputModeT mode);

	/* returns true if the internal force has been changed since
	 * the last time step */
	virtual GlobalT::RelaxCodeT RelaxSystem(void);

	/* returns 1 if DOF's are interpolants of the nodal values */
	 virtual int InterpolantDOFs(void) const;

	/* retrieve nodal unknowns */
	virtual void NodalDOFs(const iArrayT& nodes, dArray2DT& DOFs) const;

	/* weight the computational effort of every node */
	virtual void WeightNodalCost(iArrayT& weight) const;

	/* initialize/finalize time increment */
	virtual void InitStep(void);
	virtual void CloseStep(void);
	virtual void ResetStep(void); // restore last converged state
					
protected:

	/* print element group data */
	virtual void PrintControlData(ostream& out) const;

	/* initialization functions */
	virtual void SetShape(void);

	/* increment current element */
	virtual bool NextElement(void);

	/* driver for calculating output values */
	virtual void ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
	                           const iArrayT& e_codes, dArray2DT& e_values);

private:

	/* write displacement field and gradients */
	 virtual void WriteField(void); //TEMP?
	
protected:

	/* wrappers */
	nVariMatrixT<double>  fStressStiff_wrap;
	nVariMatrixT<double>  fB_wrap;
	nVariMatrixT<double>  fGradNa_wrap;
	nVariArray2DT<double> fDNa_x_wrap;
};

} // namespace Tahoe 
#endif /* _EFG_FDELASTIC_T_H_ */
