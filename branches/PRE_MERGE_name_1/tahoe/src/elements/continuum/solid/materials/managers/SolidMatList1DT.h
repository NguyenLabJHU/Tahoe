#ifndef _MATLIST_1D_T_H_
#define _MATLIST_1D_T_H_

/* base classes */
#include "StructuralMatListT.h"
#include "MaterialT.h"

/* forward declaration */
class ElasticT;
class SmallStrainT;
class FiniteStrainT;
class MultiScaleT;

class SolidMatList1DT: public StructuralMatListT, public MaterialT
{
public:

	/* constructor */
	SolidMatList1DT(int length, const ElasticT& element_group);

	/* read material data from the input stream */
	virtual void ReadMaterialData(ifstreamT& in);

private:
	
	/* errror messages */
	void Error_no_small_strain(ostream& out, int matcode) const;
	void Error_no_finite_strain(ostream& out, int matcode) const;
	void Error_no_multi_scale(ostream& out, int matcode) const;

private:

	const ElasticT&      fElementGroup;
	const SmallStrainT*  fSmallStrain;
	const FiniteStrainT* fFiniteStrain;
	const MultiScaleT*   fMultiScale;
};

#endif /* _MATLIST_1D_T_H_ */