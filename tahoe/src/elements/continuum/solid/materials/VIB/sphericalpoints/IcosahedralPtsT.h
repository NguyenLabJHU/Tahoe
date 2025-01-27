/* $Id: IcosahedralPtsT.h,v 1.5 2009-05-21 22:30:27 tdnguye Exp $ */
/* created: paklein (10/31/1997) */
#ifndef _ICO_PTS_T_H_
#define _ICO_PTS_T_H_

/* base class */
#include "SpherePointsT.h"

namespace Tahoe {

/* forward declarations */
class ifstreamT;

class IcosahedralPtsT: public SpherePointsT
{
public:

	/* constructor */
	IcosahedralPtsT(int num_points);

	/*
	 * Generate sphere points:
	 *
	 *   phi   = angle about z from x
	 *   theta = angle about x from z
	 *
	 * The final orientation is generated by applied the
	 * phi and theta rotations in succession about the local
	 * axes.
	 *
	 */
	virtual const dArray2DT& SpherePoints(double phi, double theta);
	
private:

	/* set data pointer for the specified number of integration points */
	void SetCoords(int numint);
	void SetJacobians(int numint);

private:

	/* parameters */
	int	fN;
			
};

} // namespace Tahoe 
#endif /* _ICO_PTS_T_H_ */
