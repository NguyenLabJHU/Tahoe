/* $Id: IntegratorT.h,v 1.3 2002-07-02 19:55:07 cjkimme Exp $ */
/* created: paklein (10/14/1996) */

#ifndef _CONTROLLER_T_H_
#define _CONTROLLER_T_H_

#include "Environment.h"

/* forward declarations */

namespace Tahoe {

class dArrayT;
class NodeManagerT;

/** abstract class for time integrators */
class IntegratorT
{
public:

	/** enum for implicit/explicit attribute */
	enum ImpExpFlagT {kImplicit, kExplicit};

	/** constructor */
	IntegratorT(void);

	/** destructor */
	virtual ~IntegratorT(void);

	/** \name integrator parameters */
	/*@{*/
	/** return flag indicating whether integrator is implicit or explicit */
	virtual ImpExpFlagT ImplicitExplicit(void) const = 0;

	/** return order time discretization */
	virtual int Order(void) const = 0;

	/** return order field derivative which is treated as the primary 
	 * unknown value */
	virtual int OrderOfUnknown(void) const = 0;
	/*@}*/

	/** set the time step size */
	void SetTimeStep(double dt);

	/** take control of the time at which the external force vector
	 * is formed.  Default action to simply call the NodeManagerT's
	 * FormRHS function */
	virtual void FormNodalForce(NodeManagerT* nodeboss) const;

protected:

	/** called by SetParameters to compute the specific time stepping
	 * coefficients as needed */
	virtual void ComputeParameters(void) = 0;

protected:

	/** current time step value */
	double	fdt; 
};

} // namespace Tahoe 
#endif /* _CONTROLLER_T_H_ */
