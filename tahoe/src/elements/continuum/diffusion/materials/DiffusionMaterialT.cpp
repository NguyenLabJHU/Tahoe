/* $Id: DiffusionMaterialT.cpp,v 1.7.2.1 2004-01-21 19:09:57 paklein Exp $ */
/* created: paklein (10/02/1999) */
#include "DiffusionMaterialT.h"
#include "DiffusionMatSupportT.h"

#include "StringT.h"
#include "fstreamT.h"
#include "dArrayT.h"
#include "dSymMatrixT.h"

using namespace Tahoe;

/* constructor */
DiffusionMaterialT::DiffusionMaterialT(ifstreamT& in, const DiffusionMatSupportT& support):
	ParameterInterfaceT("linear_diffusion"),
	ContinuumMaterialT(support),
	fDiffusionMatSupport(&support),
	fConductivity(NumSD()),
	fq_i(NumSD()),
	fdq_i(NumSD())	
{
	in >> fDensity;		 if (fDensity <= 0.0) throw ExceptionT::kBadInputValue;
	in >> fSpecificHeat; if (fSpecificHeat <= 0.0) throw ExceptionT::kBadInputValue;
	in >> fConductivity;
	fdq_i = 0.0;
}

DiffusionMaterialT::DiffusionMaterialT(void):
	ParameterInterfaceT("linear_diffusion"),
	fDiffusionMatSupport(NULL),
	fDensity(0.0),
	fSpecificHeat(0.0)
{

}

/* I/O functions */
void DiffusionMaterialT::Print(ostream& out) const
{
	/* inherited */
	ContinuumMaterialT::Print(out);

	out << " Density . . . . . . . . . . . . . . . . . . . . = " << fDensity      << '\n';
	out << " Specific Heat . . . . . . . . . . . . . . . . . = " << fSpecificHeat << '\n';
	out << " Conductivity:\n" << fConductivity << endl;
}

/* heat flux */
const dArrayT& DiffusionMaterialT::q_i(void)
{
	/* should be 1 row */
	fConductivity.Multx(fDiffusionMatSupport->Gradient(), fq_i, -1.0);
	return fq_i;
}

/* describe the parameters needed by the interface */
void DiffusionMaterialT::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	ContinuumMaterialT::DefineParameters(list);

	/* define parameters */
	list.AddParameter(fDensity, "density");
	list.AddParameter(fSpecificHeat, "specific_heat");
	list.AddParameter(ParameterT::Double, "conductivity");
}

/*************************************************************************
 * Protected
 *************************************************************************/

void DiffusionMaterialT::PrintName(ostream& out) const
{
	/* inherited */
	ContinuumMaterialT::PrintName(out);
	
	out << "    Linear diffusion material\n";
}
