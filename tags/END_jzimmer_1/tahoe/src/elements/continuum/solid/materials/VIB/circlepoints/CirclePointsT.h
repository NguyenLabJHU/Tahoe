/* $Id: CirclePointsT.h,v 1.3 2003-12-28 23:37:04 paklein Exp $ */
/* created: paklein (11/02/1997)                                          */
/* Base class for circular integration point generators.                  */

#ifndef _CIRCLE_PTS_T_H_
#define _CIRCLE_PTS_T_H_

/* direct members */
#include "dArray2DT.h"
#include "dMatrixT.h"
#include "dArrayT.h"


namespace Tahoe {

class CirclePointsT
{
public:

	/*
	 * Constructor
	 */
	CirclePointsT(void);

	/*
	 * Destructor
	 */
	virtual ~CirclePointsT(void);

	/*
	 * Print parameters.
	 */
	virtual void Print(ostream& out) const = 0;
	virtual void PrintName(ostream& out) const = 0;	

	/*
	 * Generate points with the given orientation angle theta.
	 */
	virtual const dArray2DT& CirclePoints(double theta) = 0;

	/*
	 * List of jacobian determinants
	 */
	const dArrayT& Jacobians(void) const;
	
protected:

	/*
	 * Orient points with given rotations (in degrees)
	 */
	void TransformPoints(double theta);
	
protected:

	/* point table */
	dArray2DT	fPoints;
	
	/* jacobians */
	dArrayT		fJacobians;

private:

	/* tranformation tensor */
	dMatrixT	fQ;
			
};

} // namespace Tahoe 
#endif /* _CIRCLE_PTS_T_H_ */