/* $Id: GradSSSolidMatList3DT.h,v 1.1 2004-09-02 18:25:04 rdorgan Exp $ */
#ifndef _GRAD_SS_SOLID_MATLIST_3D_T_H_
#define _GRAD_SS_SOLID_MATLIST_3D_T_H_

/* base classes */
#include "SSSolidMatList3DT.h"

namespace Tahoe {

/* forward declaration */
class GradSSSolidMatT;

/** materials list for 3D structural analysis */
class GradSSSolidMatList3DT: public SSSolidMatList3DT
{
public:

	/** constructor */
	GradSSSolidMatList3DT(int length, const GradSSMatSupportT& support);
	GradSSSolidMatList3DT(void);

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** return the description of the given inline subordinate parameter list */
	virtual void DefineInlineSub(const StringT& name, ParameterListT::ListOrderT& order, 
		SubListT& sub_lists) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& name) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

	/** construct the specified material or NULL if the request cannot be completed */
	GradSSSolidMatT* NewGradSSSolidMat(const StringT& name) const;

private:

	/** support for gradient enhanced small strain materials */
	const GradSSMatSupportT* fGradSSMatSupport;
};

} /* namespace Tahoe */

#endif /* _GRAD_SS_SOLID_MATLIST_3D_T_H_ */
