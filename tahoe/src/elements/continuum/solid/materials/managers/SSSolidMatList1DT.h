/* $Id: SSSolidMatList1DT.h,v 1.1.6.1 2004-07-06 06:53:52 paklein Exp $ */
#ifndef _SS_MATLIST_1D_T_H_
#define _SS_MATLIST_1D_T_H_

/* base classes */
#include "SolidMatListT.h"
#include "SolidT.h"

namespace Tahoe {

/* forward declaration */
class SSSolidMatT;

/** materials list for 1D structural analysis */
class SSSolidMatList1DT: public SolidMatListT, public SolidT
{
public:

	/** constructor */
	SSSolidMatList1DT(int length, const SSMatSupportT& support);
	SSSolidMatList1DT(void);

	/** read material data from the input stream */
	virtual void ReadMaterialData(ifstreamT& in);

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** return the description of the given inline subordinate parameter list */
	virtual void DefineInlineSub(const StringT& sub, ParameterListT::ListOrderT& order, 
		SubListT& sub_sub_list) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& list_name) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

	/** construct the specified material or NULL if the request cannot be completed */
	SSSolidMatT* NewSSSolidMat(const StringT& name) const;

private:

	/** support for small strain materials */
	const SSMatSupportT* fSSMatSupport;

	/** support for gradient enhanced small strain materials */
	const GradSSMatSupportT* fGradSSMatSupport;
};

} /* namespace Tahoe */

#endif /* _SS_MATLIST_1D_T_H_ */
