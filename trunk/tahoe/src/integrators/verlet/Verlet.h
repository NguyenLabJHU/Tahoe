#ifndef _VERLET_H_
#define _VERLET_H_

#include "Environment.h"

/* base class */
#include "ControllerT.h"

/** explicit, central differences time integrator */
class Verlet: virtual public ControllerT
{
public:

	/** constructor */
	Verlet(void) { };

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

#endif /* _VERLET_H_ */
