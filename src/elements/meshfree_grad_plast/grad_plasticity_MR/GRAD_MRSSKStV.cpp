/* created: Majid T. Manzari (04/16/2001) */
#include "GRAD_MRSSKStV.h"
#include "SSMatSupportT.h"  

#include "ElementCardT.h"
#include "StringT.h"
#include "DetCheckT.h"
#include <iostream.h>

using namespace Tahoe;

/* parameters */
const double sqrt23 = sqrt(2.0/3.0);

/* element output data */
const int kNumOutput = 14;
static const char* Labels[kNumOutput] = {
	    "chi",
	    "cohesion",
	    "Friction Angle",
	    "Dilation Angle",
	    "VM",  // Von Mises stress
	    "press", // pressurefmo
	    "loccheck",
	    "loccheckd", // localization check
	    "n1", // x1 component of normal n for contbif
	    "n2", // x2 component of normal n for contbif
	    "n3", // x3 component of normal n for contbif
	    "nd1", // x1 component of normal n for discbif	
	    "nd2", // x2 component of normal n for discbif	
	    "nd3"}; // x3 component of normal n for discbif	    

/* constructor */
GRAD_MRSSKStV::GRAD_MRSSKStV(ifstreamT& in, const SSMatSupportT& support):
	SSSolidMatT(in, support),
	IsotropicT(in),
	HookeanMatT(3),
	GRAD_MRSSNLHardT(in, NumIP(), Mu(), Lambda()),
	fStress(3),
	fFailureFunction, // kyonten
	fModulus(dSymMatrixT::NumValues(3)),
	fModulusdisc(dSymMatrixT::NumValues(3))
{

}

/* initialization */
void GRAD_MRSSKStV::Initialize(void)
{
	/* inherited */
	HookeanMatT::Initialize();
}

/* form of tangent matrix (symmetric by default) */
GlobalT::SystemTypeT GRAD_MRSSKStV::TangentType(void) const { return GlobalT::kNonSymmetric; }

/* update internal variables */
void GRAD_MRSSKStV::UpdateHistory(void)
{
	/* update if plastic */
	ElementCardT& element = CurrentElement();
	if (element.IsAllocated()) Update(element);
}

/* reset internal variables to last converged solution */
void GRAD_MRSSKStV::ResetHistory(void)
{
	/* reset if plastic */
	ElementCardT& element = CurrentElement();
	if (element.IsAllocated()) Reset(element);
}

/* print parameters */
void GRAD_MRSSKStV::Print(ostream& out) const
{
	/* inherited */
	SSSolidMatT::Print(out);
	IsotropicT::Print(out);
	GRAD_MRSSNLHardT::Print(out);
}

/* print name */
void GRAD_MRSSKStV::PrintName(ostream& out) const
{
	/* inherited */
	SSSolidMatT::PrintName(out);
	GRAD_MRSSNLHardT::PrintName(out);
	out << "    Kirchhoff-St.Venant\n";
}

/* modulus */
const dMatrixT& GRAD_MRSSKStV::c_ijkl(void)
{

	fModulus =	Moduli(CurrentElement(), CurrIP());
	return fModulus;
}

/*discontinuous modulus */
const dMatrixT& GRAD_MRSSKStV::cdisc_ijkl(void)
{
	/* elastoplastic correction */
	fModulusdisc =	ModuliDisc(CurrentElement(), CurrIP());
	return fModulusdisc;
}

/* yield function */
const double& GRAD_MRSSKStV::YieldFunction(void)
{

	fYieldFunction = YieldFunction(CurrentElement(), CurrIP());
	return fYieldFunction;
}

/* stress */
const dSymMatrixT& GRAD_MRSSKStV::s_ij(void)
{
	int ip = CurrIP();
	ElementCardT& element = CurrentElement();
	const dSymMatrixT& e_tot = e();
	const dSymMatrixT& e_els = ElasticStrain(e_tot, element, ip);
	
	/* elastic stress */
	/*HookeanStress(e_els, fStress);*/

	/* Updated Cauchy stress (return mapping) */
	fStress = StressCorrection(e_els, element, ip);
	return fStress;	
}



/*
* Test for localization using "current" values for Cauchy
* stress and the spatial tangent moduli. Returns 1 if the
* determinant of the acoustic tensor is negative and returns
* the normal for which the determinant is minimum. Returns 0
* if the determinant is positive.
*/
int GRAD_MRSSKStV::IsLocalized(dArrayT& normal)
{
        DetCheckT checker(fStress, fModulus);
        checker.SetfStructuralMatSupport(*fSSMatSupport);

        int loccheck= checker.IsLocalized(normal);
        return loccheck;
}


/* returns the strain energy density for the specified strain */
double GRAD_MRSSKStV::StrainEnergyDensity(void)
{
	return 0.;
}

/* returns the number of variables computed for nodal extrapolation
 * during for element output, ie. internal variables. Returns 0
 * by default. */
int GRAD_MRSSKStV::NumOutputVariables(void) const  { return kNumOutput; } 
void GRAD_MRSSKStV::OutputLabels(ArrayT<StringT>& labels) const
{
	/* set size */
	labels.Dimension(kNumOutput);
	
	/* copy labels */
	for (int i = 0; i < kNumOutput; i++)
		labels[i] = Labels[i];
}

void GRAD_MRSSKStV::ComputeOutput(dArrayT& output)
{
	
	/* stress tensor (load state) */
	const dSymMatrixT& stress = s_ij();

	/* pressure */
	output[5] = fStress.Trace()/3.0;

	/* deviatoric Von Mises stress */
	fStress.Deviatoric();
	double J2 = fStress.Invariant2();
	J2 = (J2 < 0.0) ? 0.0 : J2;
	output[4] = sqrt(3.0*J2);
	
	/* stress-like internal variable Chi */
	const ElementCardT& element = CurrentElement();
	if (element.IsAllocated())
	{
		output[0] = fInternal[kchi];
		output[1] = fInternal[kc];
		output[2] = fInternal[ktanphi];
		output[3] = fInternal[ktanpsi];
		output[6] = fInternal[26];
		output[7] = 0.0;
		output[8] = 0.0;
		output[9] = 0.0;
		output[10] = 0.0;
		output[11] = 0.0;
		output[12] = 0.0;
		output[13] = 0.0;
	}	
	else
	{
		output[6] = 0.0;
		output[7] = 0.0;
		output[8] = 0.0;
		output[9] = 0.0;
		output[10] = 0.0;
		output[11] = 0.0;
		output[12] = 0.0;
		output[13] = 0.0;
	}

	
}

/*************************************************************************
* Protected
*************************************************************************/

/* set modulus */
void GRAD_MRSSKStV::SetModulus(dMatrixT& modulus)
{
	IsotropicT::ComputeModuli(modulus);
}
