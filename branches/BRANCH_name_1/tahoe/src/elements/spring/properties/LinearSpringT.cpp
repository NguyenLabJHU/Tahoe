/* $Id: LinearSpringT.cpp,v 1.1.1.1.10.1 2002-06-27 18:03:54 cjkimme Exp $ */
/* created: paklein (05/28/1996)                                          */

#include "LinearSpringT.h"

#include <iostream.h>

#include "ExceptionCodes.h"
#include "fstreamT.h"

/*
* constructor
*/

using namespace Tahoe;

LinearSpringT::LinearSpringT(ifstreamT& in): RodMaterialT(in)
{
	in >> fSpringConstant;	if (fSpringConstant < 0.0) throw eBadInputValue;
}

/*
* I/O functions.
*/
void LinearSpringT::Print(ostream& out) const
{
	/* inherited */
	RodMaterialT::Print(out);

	out << " Spring constant . . . . . . . . . . . . . . . . = " << fSpringConstant << '\n';	
}

/* potential function and derivatives */
double LinearSpringT::Potential(double rmag, double Rmag) const
{
	double delta = rmag - Rmag;
	
	return 0.5*fSpringConstant*delta*delta;
}

double LinearSpringT::DPotential(double rmag, double Rmag) const
{
	return fSpringConstant*(rmag - Rmag);
}

double LinearSpringT::DDPotential(double rmag, double Rmag) const
{
#pragma unused(rmag)
#pragma unused(Rmag)

	return fSpringConstant;
}

/*************************************************************************
* Protected
*************************************************************************/

void LinearSpringT::PrintName(ostream& out) const
{
	out << "    Linear spring\n";
}
