/* $Id: GaussPtsT.h,v 1.1.1.1.10.1 2002-06-27 18:03:23 cjkimme Exp $ */
/* created: paklein (11/02/1997)                                          */

#ifndef _GAUSS_PTS_T_H_
#define _GAUSS_PTS_T_H_

/* base class */
#include "CirclePointsT.h"


namespace Tahoe {

class GaussPtsT: public CirclePointsT
{
public:

	/*
	 * Constructor
	 */
	GaussPtsT(istream& in);

	/*
	 * Print parameters.
	 */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;	

	/*
	 * Generate points with the given orientation angle theta.
	 */
	virtual const dArray2DT& CirclePoints(double theta);
	
private:

	/*
	 * Returns the correct data pointer for the specified number of
	 * integration points
	 */
	void SetCoords(int numint);
	void SetJacobians(int numint);

private:

	/* parameters */
	int	fN;
			
};

} // namespace Tahoe 
#endif /* _GAUSS_PTS_T_H_ */
