/* $Id: dArrayT.h,v 1.8 2004-03-16 05:37:14 paklein Exp $ */
/* created: paklein (08/11/1996) */
#ifndef _DARRAY_T_H_
#define _DARRAY_T_H_

/* base class */
#include "nArrayT.h"

namespace Tahoe {

class dArrayT: public nArrayT<double>
{
public:

	/** \name constructors */
	/*@{*/
	dArrayT(void);
	explicit dArrayT(int length);
	dArrayT(const dArrayT& source);

	/** construct an alias */
	dArrayT(int length, const double* p);
	/*@}*/

	/** \name assigment operators */
	/*@{*/
	dArrayT& operator=(const dArrayT& RHS); /**< assignment operator. Redimensions the array too match the source. */
	dArrayT& operator=(const double* pRHS); /**< assignment operator. Copy as many values as fit. */
	dArrayT& operator=(double value);       /**< set all elements in the array to value */
	/*@}*/

	/** L2 norm of the vector */
	double Magnitude(void) const;

	/** \name create a unit vectors */
	/*@{*/
	dArrayT& UnitVector(const dArrayT& vector);

	/** scale this */
	dArrayT& UnitVector(void);
	/*@}*/
};

/* inlines */

/* assigment operators */
inline dArrayT& dArrayT::operator=(const dArrayT& RHS)
{
	nArrayT<double>::operator=(RHS);
	return *this;
}

inline dArrayT& dArrayT::operator=(const double* pRHS)
{
	nArrayT<double>::operator=(pRHS);
	return *this;
}

inline dArrayT& dArrayT::operator=(double value)
{
	nArrayT<double>::operator=(value);
	return *this;
}

inline dArrayT& dArrayT::UnitVector(const dArrayT& vector)
{
	SetToScaled(1.0/vector.Magnitude(), vector);
	return *this;
}

inline dArrayT& dArrayT::UnitVector(void)
{
	return UnitVector(*this);
}

} // namespace Tahoe
 
#endif /* _DARRAY_T_H_ */
