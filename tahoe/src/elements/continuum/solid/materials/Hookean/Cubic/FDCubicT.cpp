/* $Id: FDCubicT.cpp,v 1.5.30.1 2004-01-21 19:10:06 paklein Exp $ */
/* created: paklein (06/11/1997) */
#include "FDCubicT.h"

using namespace Tahoe;

/* constructor */
FDCubicT::FDCubicT(ifstreamT& in, const FSMatSupportT& support):
	ParameterInterfaceT("large_strain_cubic"),
	FDHookeanMatT(in, support),
	CubicT(in)
{

}

FDCubicT::FDCubicT(void):
	ParameterInterfaceT("large_strain_cubic")
{

}

/* print parameters */
void FDCubicT::Print(ostream& out) const
{
	/* inherited */
	FDHookeanMatT::Print(out);
	CubicT::Print(out);
}

/* print name */
void FDCubicT::PrintName(ostream& out) const
{
	/* inherited */
	FDHookeanMatT::PrintName(out);
	CubicT::PrintName(out);
}

/* information about subordinate parameter lists */
void FDCubicT:: DefineParameters(ParameterListT& list) const
{
	/* inherited */
	FDHookeanMatT::DefineParameters(list);
	CubicT::DefineParameters(list);
}

/*************************************************************************
* Protected
*************************************************************************/

/* set modulus */
void FDCubicT::SetModulus(dMatrixT& modulus)
{
	CubicT::ComputeModuli(modulus);
}
