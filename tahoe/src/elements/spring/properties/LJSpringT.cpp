/* $Id: LJSpringT.cpp,v 1.4.48.1 2004-06-09 23:18:15 paklein Exp $ */
/* created: paklein (5/28/1996) */
#include "LJSpringT.h"

#include <iostream.h>
#include <math.h>

#include "Environment.h"
#include "ExceptionT.h"

#include "fstreamT.h"
#include "ThermalDilatationT.h"

using namespace Tahoe;

/* constructor  */
LJSpringT::LJSpringT(ifstreamT& in): RodMaterialT(in)
{
	in >> f_eps; if (f_eps < 0.0) ExceptionT::BadInputValue();
	in >> f_sigma; if (f_sigma < 0.0) ExceptionT::BadInputValue();
}

/*
* Returns true if the material has internal forces in the unloaded
* configuration, ie thermal strains.
*/
int LJSpringT::HasInternalStrain(void) const
{
	return 1;
	
	/*
	 * Always potentially has internal strain since the unstressed
	 * configurations is determined by the LJ potential and not the
	 * initial geometry.
	 */
}

/* potential function and derivatives */
double LJSpringT::Potential(double rmag, double Rmag) const
{
#pragma unused(Rmag)

	double a = 1.0 + fThermal->PercentElongation();
	
	double r = f_sigma*a/rmag;
	
	return 4.0*f_eps*(-pow(r,6) + pow(r,12));
}

double LJSpringT::DPotential(double rmag, double Rmag) const
{
#pragma unused(Rmag)

	double a = 1.0 + fThermal->PercentElongation();

	double r = f_sigma/rmag;

	return (4.0*f_eps/f_sigma)*(6.0*pow(r,7) - 12.0*pow(r,13));
}

double LJSpringT::DDPotential(double rmag, double Rmag) const
{
#pragma unused(Rmag)

	double a = 1.0 + fThermal->PercentElongation();

	double r = f_sigma/rmag;

	return (4.0*f_eps/f_sigma/f_sigma)*(-42.0*pow(r,8) + 156.0*pow(r,14));
}
