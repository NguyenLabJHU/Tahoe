/* $Id: dArrayT.cpp,v 1.8 2003-10-27 19:50:33 paklein Exp $ */
/* created: paklein (08/11/1996) */
#include "dArrayT.h"
#include <iostream.h>
#include <iomanip.h>
#include <math.h>
#include "toolboxConstants.h"

using namespace Tahoe;

namespace Tahoe {
template<> const bool ArrayT<dArrayT*>::fByteCopy = true; 
template<> const bool ArrayT<dArrayT>::fByteCopy = false; 
} /* namespace Tahoe */

/* constructor */
dArrayT::dArrayT(void) { }
dArrayT::dArrayT(int length): nArrayT<double>(length) { }
dArrayT::dArrayT(int length, double* p): nArrayT<double>(length,p) { }
dArrayT::dArrayT(const dArrayT& source): nArrayT<double>(source) { }

/* L2 norm of the vector */
double dArrayT::Magnitude(void) const
{
	int length = Length();
	double* p = Pointer();
	if (length == 2)
		return sqrt(p[0]*p[0] + p[1]*p[1]);
	else if (length == 3)
		return sqrt(p[0]*p[0] + p[1]*p[1] + p[2]*p[2]);
	else if (length == 1)
		return p[0];
	else
	{
		register double magsqr = 0.0;
		for (int i = 0; i < Length(); i++)
		{
			magsqr += (*p)*(*p);
			p++;
		}
		return sqrt(magsqr);
	}
}
