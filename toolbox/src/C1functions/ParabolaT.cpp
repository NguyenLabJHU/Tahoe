/* $Id: ParabolaT.cpp,v 1.3 2002-10-20 22:38:48 paklein Exp $ */
/* created: paklein (03/25/1999)                                          */

#include "ParabolaT.h"
#include <iostream.h>
#include "ExceptionT.h"
#include "dArrayT.h"

/* constructors */

using namespace Tahoe;

ParabolaT::ParabolaT(double k): fk(k) { }

/* I/O */
void ParabolaT::Print(ostream& out) const
{
	/* parameters */
	out << " U'' . . . . . . . . . . . . . . . . . . . . . . = " << fk << '\n';
}

void ParabolaT::PrintName(ostream& out) const
{
	out << "    Quadratic function\n";
}

/* returning values in groups */
dArrayT& ParabolaT::MapFunction(const dArrayT& in, dArrayT& out) const
{
	/* dimension checks */
	if (in.Length() != out.Length()) throw ExceptionT::kGeneralFail;

	double* pl = in.Pointer();
	double* pU = out.Pointer();
	
	for (int i = 0; i < in.Length(); i++)
	{
		*pU++ = fk*(*pl)*(*pl);
		pl++;
	}

	return out;
}

dArrayT& ParabolaT::MapDFunction(const dArrayT& in, dArrayT& out) const
{
	/* dimension checks */
	if (in.Length() != out.Length()) throw ExceptionT::kGeneralFail;

	double* pl  = in.Pointer();
	double* pdU = out.Pointer();
	
	for (int i = 0; i < in.Length(); i++)
		*pdU++ = fk*(*pl++);

	return out;
}

dArrayT& ParabolaT::MapDDFunction(const dArrayT& in, dArrayT& out) const
{
	/* dimension checks */
	if (in.Length() != out.Length()) throw ExceptionT::kGeneralFail;

	out = fk;
	return out;
}
