/* $Id: DPSSKStV.cpp,v 1.11 2001-08-17 00:51:03 cfoster Exp $ */
/* created: myip (06/01/1999)                                             */


#include "DPSSKStV.h"
#include "ElementCardT.h"
#include "StringT.h"
#include "DetCheckT.h"
#include <iostream.h>

/* parameters */
const double sqrt23 = sqrt(2.0/3.0);

/* element output data */
const int kNumOutput = 5;
static const char* Labels[kNumOutput] = {
	    "alpha",  // stress-like internal state variable
                      //  (isotropic linear hardening)
	       "VM",  // Von Mises stress
	    "press", // pressurefmo
	    "loccheck",
            "loccheckd"}; // localization check

/* constructor */
DPSSKStV::DPSSKStV(ifstreamT& in, const SmallStrainT& element):
	SSStructMatT(in, element),
	IsotropicT(in),
	HookeanMatT(3),
	DPSSLinHardT(in, NumIP(), Mu(), Lambda()),
	fStress(3),
	fModulus(dSymMatrixT::NumValues(3)),
        fModulusdisc(dSymMatrixT::NumValues(3))
{
 
}

/* initialization */
void DPSSKStV::Initialize(void)
{
	/* inherited */
	HookeanMatT::Initialize();
}

/* form of tangent matrix (symmetric by default) */
GlobalT::SystemTypeT DPSSKStV::TangentType(void) const { return GlobalT::kNonSymmetric; }

/* update internal variables */
void DPSSKStV::UpdateHistory(void)
{
	/* update if plastic */
	ElementCardT& element = CurrentElement();
	if (element.IsAllocated()) Update(element);
}

/* reset internal variables to last converged solution */
void DPSSKStV::ResetHistory(void)
{
	/* reset if plastic */
	ElementCardT& element = CurrentElement();
	if (element.IsAllocated()) Reset(element);
}

/* print parameters */
void DPSSKStV::Print(ostream& out) const
{
	/* inherited */
	SSStructMatT::Print(out);
	IsotropicT::Print(out);
	DPSSLinHardT::Print(out);
}

/* print name */
void DPSSKStV::PrintName(ostream& out) const
{
	/* inherited */
	SSStructMatT::PrintName(out);
	DPSSLinHardT::PrintName(out);
	out << "    Kirchhoff-St.Venant\n";
}

/* modulus */
const dMatrixT& DPSSKStV::c_ijkl(void)
{

	fModulus.SumOf(HookeanMatT::Modulus(),
		ModuliCorrection(CurrentElement(), CurrIP()));
	
	return fModulus;
}

/*discontinuous modulus */
const dMatrixT& DPSSKStV::cdisc_ijkl(void)
{
	/* elastoplastic correction */
	fModulusdisc.SumOf(HookeanMatT::Modulus(),
		ModuliCorrDisc(CurrentElement(), CurrIP()));
	return fModulusdisc;
}

/* stress */
const dSymMatrixT& DPSSKStV::s_ij(void)
{
	int ip = CurrIP();
	ElementCardT& element = CurrentElement();
	const dSymMatrixT& e_tot = e();
	const dSymMatrixT& e_els = ElasticStrain(e_tot, element, ip);

	/* elastic stress */
	HookeanStress(e_els, fStress);

	/* modify Cauchy stress (return mapping) */
	fStress += StressCorrection(e_els, element, ip);
	return fStress;	
}


/*
* Test for localization using "current" values for Cauchy
* stress and the spatial tangent moduli. Returns 1 if the
* determinant of the acoustic tensor is negative and returns
* the normal for which the determinant is minimum. Returns 0
* of the determinant is positive.
*/
int DPSSKStV::IsLocalized(dArrayT& normal)
{
        DetCheckT checker(fStress, fModulus);

        int loccheck= checker.IsLocalized(normal);
        return loccheck;
}




/* returns the strain energy density for the specified strain */
double DPSSKStV::StrainEnergyDensity(void)
{
	return HookeanEnergy(ElasticStrain(e(), CurrentElement(), CurrIP()));
}

/* returns the number of variables computed for nodal extrapolation
 * during for element output, ie. internal variables. Returns 0
 * by default. */
int DPSSKStV::NumOutputVariables(void) const  { return kNumOutput; } 
void DPSSKStV::OutputLabels(ArrayT<StringT>& labels) const
{
	/* set size */
	labels.Allocate(kNumOutput);
	
	/* copy labels */
	for (int i = 0; i < kNumOutput; i++)
		labels[i] = Labels[i];
}

void DPSSKStV::ComputeOutput(dArrayT& output)
{
	
	/* stress tensor (load state) */
	const dSymMatrixT& stress = s_ij();

	/* pressure */
	output[2] = fStress.Trace()/3.0;


	/* compute modulus */

	const dMatrixT& modulus = c_ijkl();

        /* continuous localization condition checker */
	DetCheckT checker(stress, modulus);
	dArrayT normal(stress.Rows());
	output[3] = checker.IsLocalized_SS(normal);

	/* compute discontinuous bifurcation modulus */
	const dMatrixT& modulusdisc = cdisc_ijkl();
	

	/* discontinuous localization condition checker */

	DetCheckT checkerdisc(stress, modulusdisc);
	dArrayT normaldisc(stress.Rows());
	output[4] = checkerdisc.IsLocalized_SS(normaldisc);

   
	/* deviatoric Von Mises stress */
	fStress.Deviatoric();
	double J2 = fStress.Invariant2();
	J2 = (J2 < 0.0) ? 0.0 : J2;
	output[1] = sqrt(3.0*J2);
	
	/* stress-like internal variable alpha */
	const ElementCardT& element = CurrentElement();
	if (element.IsAllocated())
	{
		output[0] = fInternal[kalpha];
		const iArrayT& flags = element.IntegerData();
		if (flags[CurrIP()] == kIsPlastic)
			output[0] -= fH_prime*fInternal[kdgamma];
	}
	else
	{
		output[0] = 0.0;
	}
}

/*************************************************************************
* Protected
*************************************************************************/

/* set modulus */
void DPSSKStV::SetModulus(dMatrixT& modulus)
{
	IsotropicT::ComputeModuli(modulus);
}

