/* $Id: EvenSpacePtsT.h,v 1.1.1.1.10.1 2002-06-27 18:03:23 cjkimme Exp $ */
/* created: paklein (11/02/1997)                                          */

#ifndef _EVENSPACE_PTS_T_H_
#define _EVENSPACE_PTS_T_H_

/* base class */
#include "CirclePointsT.h"

/* forward declarations */

namespace Tahoe {

class ifstreamT;

class EvenSpacePtsT: public CirclePointsT
{
public:

	/* constructor */
	EvenSpacePtsT(ifstreamT& in);

	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;	

	/* generate points with the given orientation angle theta */
	virtual const dArray2DT& CirclePoints(double theta);

private:

	/* parameters */
	int fNtheta;
			
};

} // namespace Tahoe 
#endif /* _EVENSPACE_PTS_T_H_ */