/* $Id: dRangeArrayT.h,v 1.3 2002-07-02 19:56:50 cjkimme Exp $ */
/* created: paklein (12/02/1996) */

#ifndef _DRANGEARRAY_T_H_
#define _DRANGEARRAY_T_H_

/* base class */
#include "dArrayT.h"

/* forward declarations */

namespace Tahoe {

class dArray2DT;

/** Class to identify the interval numbers in a list of floating
 * point numbers. Values defining the intervals must be provided
 * in ascending order. */
class dRangeArrayT: private dArrayT
{
public:

	/* constructor */
	dRangeArrayT(void);
	dRangeArrayT(const dArrayT& values);
	dRangeArrayT(int colnum, const dArray2DT& values2D);

	/** output operator */
	friend ostream& operator<<(ostream& out, const dRangeArrayT& array);

	/** set values. \param values array of points defining subintervals */
	void SetValues(const dArrayT& values);

	/** find the interval containing the given value. The Range is
	 * an integer { 0...Length() }, where 0 means the value is less
	 * than the first array value, Length() means it's greater than
	 * the last, and in general:
	 *
	 *		i(x): array[i-1] < x < array[i]
	 *
	 */
	int Range(double value) const;
	
	/** length of the array */
	int Length(void) const;
	
	/** make element accessor public */
	dArrayT::operator[];
	
private:
	
	/** returns 1 if the data is in ascending order */
	int IsSequential(void) const;
	
};

/* inlines */

/*  still allow length checks */
inline int dRangeArrayT::Length(void) const { return dArrayT::Length(); }	

} // namespace Tahoe 
#endif /* _DRANGEARRAY_T_H_ */
