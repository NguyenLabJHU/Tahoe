/* $Id: HarmonicPairT.cpp,v 1.3 2003-03-31 23:09:14 paklein Exp $ */
#include "HarmonicPairT.h"
#include <iostream.h>

using namespace Tahoe;

/* initialize static parameters */
double HarmonicPairT::sR0 = 0.0;
double HarmonicPairT::sK = 0.0;

/* constructor */
HarmonicPairT::HarmonicPairT(double mass, double R0, double K):
	fR0(R0),
	fK(K)
{
	/* assume nearest neighbor - 10x of equilibrium spacing */
	SetRange(10.0*fR0);
	SetMass(mass);
}

/* return a pointer to the energy function */
PairPropertyT::EnergyFunction HarmonicPairT::getEnergyFunction(void)
{
	/* copy my data to static */
	sR0 = fR0;
	sK = fK;

	/* return function pointer */
	return HarmonicPairT::Energy;
}

PairPropertyT::ForceFunction HarmonicPairT::getForceFunction(void)
{
	/* copy my data to static */
	sR0 = fR0;
	sK = fK;

	/* return function pointer */
	return HarmonicPairT::Force;
}

PairPropertyT::StiffnessFunction HarmonicPairT::getStiffnessFunction(void)
{
	/* copy my data to static */
	sR0 = fR0;
	sK = fK;

	/* return function pointer */
	return HarmonicPairT::Stiffness;
}

/* write properties to output */
void HarmonicPairT::Write(ostream& out) const
{
	/* inherited */
	PairPropertyT::Write(out);
	out << " Equilibrium bond length . . . . . . . . . . . . = " << fR0 << '\n';
	out << " Potential well curvature. . . . . . . . . . . . = " << fK << '\n';	
}

/***********************************************************************
 * Private
 ***********************************************************************/

double HarmonicPairT::Energy(double r_ab, double* data_a, double* data_b)
{
#pragma unused(data_a)
#pragma unused(data_b)

	double dr = r_ab - sR0;
	return 0.5*sK*dr*dr;
}

double HarmonicPairT::Force(double r_ab, double* data_a, double* data_b)
{
#pragma unused(data_a)
#pragma unused(data_b)

	double dr = r_ab - sR0;
	return sK*dr;
}

double HarmonicPairT::Stiffness(double r_ab, double* data_a, double* data_b)
{
#pragma unused(data_a)
#pragma unused(data_b)
#pragma unused(r_ab)

	return sK;
}