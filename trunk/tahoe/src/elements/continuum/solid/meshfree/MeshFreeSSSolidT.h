/* $Id: MeshFreeSSSolidT.h,v 1.6 2002-07-05 22:28:04 paklein Exp $ */
/* created: paklein (09/11/1998) */

#ifndef _MF_SMALLSTRAIN_T_H_
#define _MF_SMALLSTRAIN_T_H_

/* base classes */
#include "SmallStrainT.h"
#include "MeshFreeFractureSupportT.h"

/* direct members */
#include "nVariMatrixT.h"

namespace Tahoe {

/** small strain elasticity with MLS shapefunctions for the
 * field (displacement) representation
 * \note clean up code governing when crack growth algorithm
 * is used, initiation criteria, etc. (PAK 09/28/1999) */
class MeshFreeSSSolidT: public SmallStrainT, public MeshFreeFractureSupportT
{
public:

	/* constructor */
	MeshFreeSSSolidT(const ElementSupportT& support, const FieldT& field);
	
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
	
private:

	/* dynamic wrapper */
	nVariMatrixT<double> fB_wrap;
	
	/* field ready flag */
	bool fFieldSet;
};

} // namespace Tahoe 
#endif /* _MF_SMALLSTRAIN_T_H_ */
