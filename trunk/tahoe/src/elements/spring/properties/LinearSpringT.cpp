/* $Id: LinearSpringT.cpp,v 1.3 2002-08-10 02:37:59 paklein Exp $ */
/* created: paklein (05/28/1996) */
#include "LinearSpringT.h"

#include <iostream.h>

#include "ExceptionCodes.h"
#include "fstreamT.h"
#include "ThermalDilatationT.h"

using namespace Tahoe;

/* constructor */
LinearSpringT::LinearSpringT(ifstreamT& in): RodMaterialT(in)
{
	in >> fSpringConstant;	if (fSpringConstant < 0.0) throw eBadInputValue;
}

/* I/O functions */
void LinearSpringT::Print(ostream& out) const
{
	/* inherited */
	RodMaterialT::Print(out);

	out << " Spring constant . . . . . . . . . . . . . . . . = " << fSpringConstant << '\n';	
}

void LinearSpringT::PrintName(ostream& out) const
{
	out << "    Linear spring\n";
}

/* potential function and derivatives */
double LinearSpringT::Potential(double rmag, double Rmag) const
{
	/* imposed strain */
	if (fThermal->IsActive()) Rmag *= (1.0 + fThermal->PercentElongation());
	double delta = rmag - Rmag;	
	return 0.5*fSpringConstant*delta*delta;
}

double LinearSpringT::DPotential(double rmag, double Rmag) const
{
	/* imposed strain */
	if (fThermal->IsActive()) Rmag *= (1.0 + fThermal->PercentElongation());
	return fSpringConstant*(rmag - Rmag);
}

double LinearSpringT::DDPotential(double rmag, double Rmag) const
{
#pragma unused(rmag)
#pragma unused(Rmag)

	return fSpringConstant;
}
