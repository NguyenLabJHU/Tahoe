/* $Id: NL_E_MatT.cpp,v 1.6.46.2 2004-06-14 04:56:36 paklein Exp $ */
/* created: paklein (06/13/1997) */
#include "NL_E_MatT.h"

using namespace Tahoe;

/* constructors */
NL_E_MatT::NL_E_MatT(ifstreamT& in, const FSMatSupportT& support):
	ParameterInterfaceT("large_strain_E_material"),
	FSSolidMatT(in, support),
	fE(NumSD()),
	fPK2(NumSD()),
	fModuli(dSymMatrixT::NumValues(NumSD()))
{

}

NL_E_MatT::NL_E_MatT(void):
	ParameterInterfaceT("large_strain_E_material")
{

}

/* spatial description */
const dMatrixT& NL_E_MatT::c_ijkl(void)
{
	/* strain */
	Compute_E(fE);

	/* derived class function */
	ComputeModuli(fE, fModuli);
	
	/* material -> spatial */
	const dMatrixT& Fmat = F();
	fModuli.SetToScaled(1.0/Fmat.Det(), PushForward(Fmat, fModuli));	
	return fModuli;
}
	
const dSymMatrixT& NL_E_MatT::s_ij(void)
{
	/* strain */
	Compute_E(fE);

	/* derived class function */
	ComputePK2(fE, fPK2);

	/* material -> spatial */
	const dMatrixT& Fmat = F();
	fPK2.SetToScaled(1.0/Fmat.Det(), PushForward(Fmat, fPK2));	
	return fPK2;
}

/* material description */
const dMatrixT& NL_E_MatT::C_IJKL(void)
{
	/* strain */
	Compute_E(fE);

	/* derived class function */
	ComputeModuli(fE, fModuli);
	return fModuli;
}
	
const dSymMatrixT& NL_E_MatT::S_IJ(void)
{
	/* strain */
	Compute_E(fE);

	/* derived class function */
	ComputePK2(fE, fPK2);
	 return fPK2;
}

/* returns the strain energy density for the specified strain */
double NL_E_MatT::StrainEnergyDensity(void)
{
	/* strain */
	Compute_E(fE);

	/* derived class function */
	return ComputeEnergyDensity(fE);
}

/* accept parameter list */
void NL_E_MatT::TakeParameterList(const ParameterListT& list)
{
	/* inherited */
	FSSolidMatT::TakeParameterList(list);

	/* dimension work space */
	fE.Dimension(NumSD());
	fPK2.Dimension(NumSD());
	fModuli.Dimension(dSymMatrixT::NumValues(NumSD()));
}
