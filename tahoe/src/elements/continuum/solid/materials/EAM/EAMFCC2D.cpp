/* $Id: EAMFCC2D.cpp,v 1.9.2.1 2004-07-06 06:53:26 paklein Exp $ */
/* created: paklein (12/09/1996) */
#include "EAMFCC2D.h"

#include "EAMFCC3DSym.h"
#include "dMatrixT.h"

#include <math.h>

using namespace Tahoe;

/* material parameters */
const int knsd = 2;

//TEMP
#pragma message("rename me")

/* constructor */
EAMFCC2D::EAMFCC2D(void):
	ParameterInterfaceT("FCC_EAM_2D"),
	fEAM(NULL)
{

}

/* destructor */
EAMFCC2D::~EAMFCC2D(void) { delete fEAM; }

/* describe the parameters needed by the interface */
void EAMFCC2D::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	NL_E_MatT::DefineParameters(list);
	
	/* 2D option must be plain stress */
	ParameterT& constraint = list.GetParameter("constraint_2D");
	constraint.SetDefault(kPlaneStrain);
}

/* describe the parameters needed by the interface */
void EAMFCC2D::DefineSubs(SubListT& sub_list) const
{
	/* inherited */
	NL_E_MatT::DefineSubs(sub_list);

	/* Cauchy-Born EAM parameters */
	sub_list.AddSub("FCC_EAM_Cauchy-Born");
}

/* a pointer to the ParameterInterfaceT of the given subordinate */
ParameterInterfaceT* EAMFCC2D::NewSub(const StringT& list_name) const
{
	if (list_name == "FCC_EAM_Cauchy-Born")
		return new EAMFCC3DSym;
	else /* inherited */
		return NL_E_MatT::NewSub(list_name);
}

/* accept parameter list */
void EAMFCC2D::TakeParameterList(const ParameterListT& list)
{
	/* inherited */
	NL_E_MatT::TakeParameterList(list);

	/* construct Cauchy-Born EAM solver */
	fEAM = new EAMFCC3DSym;
	fEAM->TakeParameterList(list.GetList("FCC_EAM_Cauchy-Born"));
}

/*************************************************************************
 * Private
 *************************************************************************/

void EAMFCC2D::ComputeModuli(const dSymMatrixT& E, dMatrixT& moduli)
{
	/* EAM solver */
	fEAM->Moduli(moduli, E);
}

/* returns the stress corresponding to the given strain - the strain
* is the reduced index Green strain vector, while the stress is the
* reduced index 2nd Piola-Kirchhoff stress vector */
void EAMFCC2D::ComputePK2(const dSymMatrixT& E, dSymMatrixT& PK2)
{
	/* EAM solver */
	fEAM->SetStress(E, PK2);
}

/* returns the strain energy density for the specified strain */
double EAMFCC2D::ComputeEnergyDensity(const dSymMatrixT& E)
{
	/* EAM solver */
	return fEAM->EnergyDensity(E);
}
