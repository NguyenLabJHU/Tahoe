/* $Id: LinearDecreaseT.cpp,v 1.2.2.1 2002-10-17 01:46:10 paklein Exp $ */
/* created: paklein (03/25/1999)                                          */

#include "LinearDecreaseT.h"
#include <iostream.h>
#include "ExceptionT.h"
#include "dArrayT.h"

/* constructors */

using namespace Tahoe;

LinearDecreaseT::LinearDecreaseT(double A, double L): 
  fA(A),
  fL(L)
{ }

/* I/O */
void LinearDecreaseT::Print(ostream& out) const
{
	/* parameters */
	out <<"A: "<< fA << " ...... L: "<<fL<< '\n';
}

void LinearDecreaseT::PrintName(ostream& out) const
{
        out << "Function .............  1-A(x/L)"<<'\n';
}

/* returning values in groups */
dArrayT& LinearDecreaseT::MapFunction(const dArrayT& in, dArrayT& out) const
{
	/* dimension checks */
	if (in.Length() != out.Length()) throw ExceptionT::kGeneralFail;

	double* pl = in.Pointer();
	double* pU = out.Pointer();
	
	for (int i = 0; i < in.Length(); i++)
	{
		*pU++ = (1-fA*(*pl)/fL);
		pl++;
	}

	return out;
}

dArrayT& LinearDecreaseT::MapDFunction(const dArrayT& in, dArrayT& out) const
{
	/* dimension checks */
	if (in.Length() != out.Length()) throw ExceptionT::kGeneralFail;

	out = -fA/fL;

	return out;
}

dArrayT& LinearDecreaseT::MapDDFunction(const dArrayT& in, dArrayT& out) const
{
	/* dimension checks */
	if (in.Length() != out.Length()) throw ExceptionT::kGeneralFail;

	out = 0;
	return out;
}
