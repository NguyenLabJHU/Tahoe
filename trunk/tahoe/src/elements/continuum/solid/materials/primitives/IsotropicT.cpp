/* $Id: IsotropicT.cpp,v 1.4 2002-07-02 19:56:22 cjkimme Exp $ */
/* created: paklein (06/10/1997)                                          */

#include "IsotropicT.h"

#include <iostream.h>

#include "dMatrixT.h"
#include "fstreamT.h"

/* constructor */

using namespace Tahoe;

IsotropicT::IsotropicT(ifstreamT& in)
{
	double E, nu;
	in >> E >> nu;
	try { Set_E_nu(E, nu); }
	catch (int exception) { throw eBadInputValue; }
}

IsotropicT::IsotropicT(void)
{
	try { Set_E_nu(0.0, 0.0); }
	catch (int exception) { throw eBadInputValue; }
}

/* set moduli */
void IsotropicT::Set_E_nu(double E, double nu)
{
	fYoung = E;
	fPoisson = nu;

	/* checks */
	if (fYoung < 0.0) throw eGeneralFail;
	if (fPoisson > 0.5 || fPoisson < -1.0) throw eGeneralFail;
	
	/* compute remaining moduli */
	fMu     = 0.5*fYoung/(1.0 + fPoisson);
	fLambda = 2.0*fMu*fPoisson/(1.0 - 2.0*fPoisson);
	fKappa  = fLambda + 2.0/3.0*fMu;
}

void IsotropicT::Set_mu_kappa(double mu, double kappa)
{
	fMu = mu;
	fKappa = kappa;

	/* checks */
	if (fMu < 0.0 || fKappa < 0.0) throw eGeneralFail;

	/* set moduli */
	fYoung = (9.0*fKappa*fMu)/(3.0*fKappa + fMu);
	fPoisson = (3.0*fKappa - 2.0*fMu)/(6.0*fKappa + 2.0*fMu);
	fLambda = 2.0*fMu*fPoisson/(1.0 - 2.0*fPoisson);
}

/* I/O operators */
void IsotropicT::Print(ostream& out) const
{
	out << " Young's modulus . . . . . . . . . . . . . . . . = " << fYoung   << '\n';
	out << " Poisson's ratio . . . . . . . . . . . . . . . . = " << fPoisson << '\n';
	out << " Shear modulus . . . . . . . . . . . . . . . . . = " << fMu      << '\n';
	out << " Bulk modulus. . . . . . . . . . . . . . . . . . = " << fKappa   << '\n';
}

/*************************************************************************
* Protected
*************************************************************************/

/* compute the symetric Cij reduced index matrix */
void IsotropicT::ComputeModuli(dMatrixT& moduli) const
{
	if (moduli.Rows() == 6)
	{
		double mu = Mu();
		double lambda = Lambda();
		moduli = 0.0;
		moduli(2,2) = moduli(1,1) = moduli(0,0) = lambda + 2.0*mu;
		moduli(1,2) = moduli(0,1) = moduli(0,2) = lambda;
		moduli(5,5) = moduli(4,4) = moduli(3,3) = mu;

		/* symmetric */
		moduli.CopySymmetric();
	}
	else
	{
		cout << "\n IsotropicT::ComputeModuli: for 3D only" << endl;
		throw eSizeMismatch;
	}
}

void IsotropicT::ComputeModuli2D(dMatrixT& moduli, 
	Material2DT::ConstraintOptionT constraint) const
{
	if (moduli.Rows() == 3)
	{
		double mu = Mu();
		double lambda = Lambda();

		/* plane stress correction */
		if (constraint == Material2DT::kPlaneStress) 
			lambda *= 2.0*mu/(lambda + 2.0*mu);	
		
		moduli = 0.0;
		moduli(1,1) = moduli(0,0) = lambda + 2.0*mu;
		moduli(0,1) = moduli(1,0) = lambda;
		moduli(2,2) = mu;
	}
	else 
		throw eSizeMismatch;
}

/* scale factor for constrained dilatation */
double IsotropicT::DilatationFactor2D(Material2DT::ConstraintOptionT constraint) const
{
	if (constraint == Material2DT::kPlaneStrain)
		return 1.0 + Poisson();
	else
		return 1.0;
}
