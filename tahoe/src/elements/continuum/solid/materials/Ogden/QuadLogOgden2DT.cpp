/* $Id: QuadLogOgden2DT.cpp,v 1.1 2001-02-20 00:26:51 paklein Exp $ */
/* created: paklein (02/18/2001)                                          */
/* plane strain QuadLog with Ogden principal stretch formulation          */

#include "QuadLogOgden2DT.h"
#include <math.h>
#include <iostream.h>
#include "fstreamT.h"

/* constructor */
QuadLogOgden2DT::QuadLogOgden2DT(ifstreamT& in, const ElasticT& element):
	OgdenIsotropicT(in, element),
	Material2DT(in, kPlaneStrain),
	flogE(2)
{
	fDensity *= fThickness;
	
	/* read modulus */
	double E, nu;
	in >> E >> nu;
	IsotropicT::Set_E_nu(E, nu);
}

/* print name */
void QuadLogOgden2DT::PrintName(ostream& out) const
{
	/* inherited */
	OgdenIsotropicT::PrintName(out);
	out << "    Plane Strain\n";
}

/* strain energy density */
double QuadLogOgden2DT::StrainEnergyDensity(void)
{
	/* principal stretches */
	C().PrincipalValues(fEigs);
	
	/* log strain */
	flogE[0] = 0.5*log(fEigs[0]);
	flogE[1] = 0.5*log(fEigs[1]);

	return 0.5*Lambda()*pow(flogE.Sum(), 2.0) + 
	           Mu()*dArrayT::Dot(flogE, flogE);
}

/*************************************************************************
* Protected
*************************************************************************/

/* principal values given principal values of the stretch tensors,
 * i.e., the principal stretches squared */
void QuadLogOgden2DT::dWdE(const dArrayT& eigenstretch2, dArrayT& eigenstress)
{
	/* log strain */
	flogE[0] = 0.5*log(eigenstretch2[0]);
	flogE[1] = 0.5*log(eigenstretch2[1]);
	double sum = flogE[0] + flogE[1];

	/* stress */
	eigenstress[0] = (Lambda()*sum + 2.0*Mu()*flogE[0])/eigenstretch2[0];
	eigenstress[1] = (Lambda()*sum + 2.0*Mu()*flogE[1])/eigenstretch2[1];
}

void QuadLogOgden2DT::ddWddE(const dArrayT& eigenstretch2, dArrayT& eigenstress,
	dSymMatrixT& eigenmod)
{
	/* log strain */
	flogE[0] = 0.5*log(eigenstretch2[0]);
	flogE[1] = 0.5*log(eigenstretch2[1]);
	double sum = flogE[0] + flogE[1];

	/* stress */
	eigenstress[0] = (Lambda()*sum + 2.0*Mu()*flogE[0])/eigenstretch2[0];
	eigenstress[1] = (Lambda()*sum + 2.0*Mu()*flogE[1])/eigenstretch2[1];
	
	/* ddWddLogE */
	eigenmod = Lambda();
	eigenmod.PlusIdentity(2.0*Mu());

	/* moduli */
	eigenmod(0,0) = eigenmod(0,0)/eigenstretch2[0]/eigenstretch2[0] -
		eigenstress[0]*2.0/eigenstretch2[0];
	eigenmod(1,1) = eigenmod(1,1)/eigenstretch2[1]/eigenstretch2[1] -
		eigenstress[1]*2.0/eigenstretch2[1];
	eigenmod(0,1) = eigenmod(0,1)/eigenstretch2[0]/eigenstretch2[1];
}
