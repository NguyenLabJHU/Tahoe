/* $Id: ExplicitCD.h,v 1.1 2002-04-02 23:20:41 paklein Exp $ */
/* created: paklein (10/14/1996) */

#ifndef _EXPLICIT_CD_H_
#define _EXPLICIT_CD_H_

#include "Environment.h"

/* base class */
#include "ControllerT.h"

/** explicit, central differences time integrator */
class ExplicitCD: virtual public ControllerT
{
public:

	/** constructor */
	ExplicitCD(void) { };

	/** \name integrator parameters */
	/*@{*/
	/** return flag indicating whether integrator is implicit or explicit */
	virtual ImpExpFlagT ImplicitExplicit(void) const { return kExplicit; };

	/** return order time discretization */
	virtual int Order(void) const { return 2; };

	/** return order field derivative which is treated as the primary 
	 * unknown value */
	virtual int OrderOfUnknown(void) const { return 2; };
	/*@}*/
};

#endif /* _EXPLICIT_CD_H_ */