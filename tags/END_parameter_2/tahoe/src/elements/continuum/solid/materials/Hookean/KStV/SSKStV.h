/* $Id: SSKStV.h,v 1.4.32.2 2004-02-18 16:33:44 paklein Exp $ */
/* created: paklein (06/10/1997) */
#ifndef _SS_KSTV_H_
#define _SS_KSTV_H_

/* base classes */
#include "SSHookeanMatT.h"
#include "IsotropicT.h"

namespace Tahoe {

class SSKStV: public SSHookeanMatT, public IsotropicT
{
public:

	/* constructor */
	SSKStV(ifstreamT& in, const SSMatSupportT& support);
	SSKStV(void);

	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& list_name) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

protected:

	/* set modulus */
	virtual void SetModulus(dMatrixT& modulus);
};

} // namespace Tahoe 
#endif /* _SS_KSTV_H_ */