#ifndef _GEAR6_CONTROLLER_H_
#define _GEAR6_CONTROLLER_H_

#include "Environment.h"

/* base classes */
#include "nGear6.h"
#include "eGear6.h"

/* forward declarations */
#include "ios_fwd_decl.h"

namespace Tahoe {

/** controller for an explicit 6th order Gear predictor corrector
 * time integration algorithm */
class Gear6Integrator: public nGear6, public eGear6
{
public:

	/** constructor */
	Gear6Integrator(ostream& out);
	  	
protected:  	
	
	/** recalculate time stepping constants */
	virtual void ComputeParameters(void);
};

} // namespace Tahoe

#endif /* _GEAR6_CONTROLLER_H_ */
