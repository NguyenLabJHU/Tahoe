/* $Id: CSEIsoT.h,v 1.6 2002-11-26 00:17:34 cjkimme Exp $ */
/* created: paklein (11/19/1997) */

#ifndef _CSE_ISO_T_H_
#define _CSE_ISO_T_H_

/* base class */
#include "CSEBaseT.h"

/* direct members */
#include "pArrayT.h"

namespace Tahoe {

/* forward declarations */
class C1FunctionT;

/** Cohesive surface elements with scalar traction potentials,
 * i.e., the traction potential is a function of the gap magnitude,
 * or effective gap magnitude only. */
class CSEIsoT: public CSEBaseT
{
public:

#ifndef _SIERRA_TEST_
	/* constructor */
	CSEIsoT(const ElementSupportT& support, const FieldT& field);
#else
	CSEIsoT(const ElementSupportT& support);
#endif

	/* form of tangent matrix */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/* initialize class data */
	virtual void Initialize(void);

protected:

	/* tangent matrix */
	virtual void LHSDriver(void);

	/* force vector */
	virtual void RHSDriver(void);

	/* compute output values */
	virtual void ComputeOutput(const iArrayT& n_codes, dArray2DT& n_values,
		const iArrayT& e_codes, dArray2DT& e_values);
	
protected:

	/* cohesive surface potentials */
	pArrayT<C1FunctionT*> fSurfPots;
};

} // namespace Tahoe 
#endif /* _CSE_ISO_T_H_ */
