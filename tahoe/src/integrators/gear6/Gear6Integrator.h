/* $Id: Gear6Integrator.h,v 1.2.40.1 2004-01-28 01:34:06 paklein Exp $ */
#ifndef _GEAR_06_CONTROLLER_H_
#define _GEAR_06_CONTROLLER_H_

/* base classes */
#include "nGear6.h"
#include "eGear6.h"

namespace Tahoe {

/** controller for an explicit 6th order Gear predictor corrector
 * time integration algorithm */
class Gear6Integrator: public nGear6, public eGear6
{
public:

	/** constructor */
	Gear6Integrator(void);
	  	
protected:  	
	
	/** recalculate time stepping constants */
	virtual void ComputeParameters(void);
};

} // namespace Tahoe

#endif /* _GEAR_06_CONTROLLER_H_ */
