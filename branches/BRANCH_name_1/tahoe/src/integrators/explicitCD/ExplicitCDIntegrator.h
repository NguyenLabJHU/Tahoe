/* $Id: ExplicitCDIntegrator.h,v 1.2.4.1 2002-06-27 18:02:30 cjkimme Exp $ */
/* created: paklein (03/23/1997) */

#ifndef _EXP_CD_CONTROLLER_H_
#define _EXP_CD_CONTROLLER_H_

#include "Environment.h"

/* base classes */
#include "nExplicitCD.h"
#include "eExplicitCD.h"

/* forward declarations */
#include "ios_fwd_decl.h"

/** controller for an explicit 2nd order accurate, central difference
 * time-stepping algorithm */

namespace Tahoe {

class ExplicitCDIntegrator: public nExplicitCD, public eExplicitCD
{
public:

	/** constructor */
	ExplicitCDIntegrator(ostream& out);
	  	
protected:  	
	
	/** recalculate time stepping constants */
	virtual void ComputeParameters(void);
};

} // namespace Tahoe 
#endif /* _EXP_CD_CONTROLLER_H_ */