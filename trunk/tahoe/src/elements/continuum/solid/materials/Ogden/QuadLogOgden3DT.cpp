/* $Id: QuadLogOgden3DT.cpp,v 1.4 2002-11-14 17:06:07 paklein Exp $ */
/* created: paklein (02/17/2001) */
#include "QuadLogOgden3DT.h"

#include <iostream.h>
#include <math.h>
#include "fstreamT.h"

using namespace Tahoe;

/* constructor */
QuadLogOgden3DT::QuadLogOgden3DT(ifstreamT& in, const FDMatSupportT& support):
	OgdenIsotropicT(in, support),
	flogE(3)
{
	/* read modulus */
	double E, nu;
	in >> E >> nu;
	IsotropicT::Set_E_nu(E, nu);
}

void QuadLogOgden3DT::PrintName(ostream& out) const
{
	/* inherited */
	OgdenIsotropicT::PrintName(out);
	out << "    Quad Log\n";
}

/* strain energy density */
double QuadLogOgden3DT::StrainEnergyDensity(void)
{
	/* stretch */
	Compute_C(fC);

	/* principal stretches */
	fC.PrincipalValues(fEigs);
	
	/* log strain */
	flogE[0] = 0.5*log(fEigs[0]);
	flogE[1] = 0.5*log(fEigs[1]);
	flogE[2] = 0.5*log(fEigs[2]);

	return 0.5*Lambda()*pow(flogE.Sum(), 2.0) + 
	           Mu()*dArrayT::Dot(flogE, flogE);
}

/***********************************************************************
* Protected
***********************************************************************/

/* principal values given principal values of the stretch tensors,
 * i.e., the principal stretches squared */
void QuadLogOgden3DT::dWdE(const dArrayT& eigenstretch2, dArrayT& eigenstress)
{
	/* log strain */
	flogE[0] = 0.5*log(eigenstretch2[0]);
	flogE[1] = 0.5*log(eigenstretch2[1]);
	flogE[2] = 0.5*log(eigenstretch2[2]);
	double sum = flogE[0] + flogE[1] + flogE[2];

	/* stress */
	eigenstress[0] = (Lambda()*sum + 2.0*Mu()*flogE[0])/eigenstretch2[0];
	eigenstress[1] = (Lambda()*sum + 2.0*Mu()*flogE[1])/eigenstretch2[1];
	eigenstress[2] = (Lambda()*sum + 2.0*Mu()*flogE[2])/eigenstretch2[2];
}

void QuadLogOgden3DT::ddWddE(const dArrayT& eigenstretch2, dArrayT& eigenstress,
	dSymMatrixT& eigenmod)
{
	/* log strain */
	flogE[0] = 0.5*log(eigenstretch2[0]);
	flogE[1] = 0.5*log(eigenstretch2[1]);
	flogE[2] = 0.5*log(eigenstretch2[2]);
	double sum = flogE[0] + flogE[1] + flogE[2];

	/* stress */
	eigenstress[0] = (Lambda()*sum + 2.0*Mu()*flogE[0])/eigenstretch2[0];
	eigenstress[1] = (Lambda()*sum + 2.0*Mu()*flogE[1])/eigenstretch2[1];
	eigenstress[2] = (Lambda()*sum + 2.0*Mu()*flogE[2])/eigenstretch2[2];
	
	/* ddWddLogE */
	eigenmod = Lambda();
	eigenmod.PlusIdentity(2.0*Mu());

	/* moduli */
	eigenmod(0,0) = eigenmod(0,0)/eigenstretch2[0]/eigenstretch2[0] -
		eigenstress[0]*2.0/eigenstretch2[0];
	eigenmod(1,1) = eigenmod(1,1)/eigenstretch2[1]/eigenstretch2[1] -
		eigenstress[1]*2.0/eigenstretch2[1];
	eigenmod(2,2) = eigenmod(2,2)/eigenstretch2[2]/eigenstretch2[2] -
		eigenstress[2]*2.0/eigenstretch2[2];

	eigenmod(0,1) = eigenmod(0,1)/eigenstretch2[0]/eigenstretch2[1];
	eigenmod(0,2) = eigenmod(0,2)/eigenstretch2[0]/eigenstretch2[2];
	eigenmod(1,2) = eigenmod(1,2)/eigenstretch2[1]/eigenstretch2[2];
}
