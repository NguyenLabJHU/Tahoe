#ifndef _GEAR_06_H_
#define _GEAR_06_H_

#include "Environment.h"

/* base class */
#include "ControllerT.h"

namespace Tahoe {

/** Gear time integrator. Base class for the Gear predictor-corrector 
 * time integrator with a local trunction error of 
 * \f$ \Delta t^6 \f$. */
class Gear6: virtual public ControllerT
{
public:

	/** constructor */
	Gear6(void) { };

	/** \name integrator parameters */
	/*@{*/
	/** return flag indicating whether integrator is implicit or explicit */
	virtual ImpExpFlagT ImplicitExplicit(void) const { return kExplicit; };

	/** return order time discretization */
	virtual int Order(void) const { return 5; };

	/** return order field derivative which is treated as the primary 
	 * unknown value */
	virtual int OrderOfUnknown(void) const { return 2; };
	/*@}*/
};

} // namespace Tahoe

#endif /* _GEAR_06_H_ */
