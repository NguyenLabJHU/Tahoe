/* $Id: eTrapezoid.h,v 1.2.4.1 2002-06-27 18:02:32 cjkimme Exp $ */
/* created: paklein (10/03/1999) */

#ifndef _E_TRAPEZOID_H_
#define _E_TRAPEZOID_H_

/* base classes */
#include "Trapezoid.h"
#include "eControllerT.h"


namespace Tahoe {

class eTrapezoid: public virtual Trapezoid, public eControllerT
{
public:

	/** constructor */
	eTrapezoid(void);

	/** \name elements of the effective mass matrix
	 * returns 1 if the algorithm requires M, C, or K and sets const equal
	 * to the coefficient for the linear combination of components in the
	 * element effective mass matrix */
	/*@{*/
	virtual int FormM(double& constM) const;
	virtual int FormC(double& constC) const;
	virtual int FormK(double& constK) const;
	/*@}*/

	/** \name elements of the residual
	 * components of the internal force vector */
	/*@{*/
	virtual int FormMa(double& constMa) const;
	virtual int FormCv(double& constCv) const;
	virtual int FormKd(double& constKd) const;
	/*@}*/

protected:  	
	
	/** recalculate constants */
	virtual void eComputeParameters(void);

private:
	
	/** \name effective mass coefficients */
	/*@{*/
	double	fconstC;
	double	fconstK;
	/*@}*/
};

} // namespace Tahoe 
#endif /* _E_TRAPEZOID_H_ */
