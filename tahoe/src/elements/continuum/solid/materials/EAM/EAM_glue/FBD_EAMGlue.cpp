/* $Id: FBD_EAMGlue.cpp,v 1.3.50.1 2004-06-16 00:31:53 paklein Exp $ */
/* created: paklein (01/30/2000) */
#include "FBD_EAMGlue.h"

#include "fstreamT.h"
#include "PhiSplineT.h"

using namespace Tahoe;

/* constructor */
FBD_EAMGlue::FBD_EAMGlue(CBLatticeT& lattice, int nsd, ifstreamT& in):
	EAM(lattice, nsd)
{
	/* clear comment line */
	in.clear_line();

	/* lattice information */
	int z;
	in >> z >> fMass >> fLatticeParameter;
	in.clear_line();
	
	/* table dimensions */
	int np, nr;
	double dp, dr, r_cut;
	in >> np >> dp >> nr >> dr >> r_cut;
	if (np < 2   ||
	    dp < 0.0 ||
	    nr < 2   ||
	    dr < 0.0 ||
	 r_cut < 0.0) throw ExceptionT::kBadInputValue;
	
	/* work space */
	dArrayT tmp;
	dArray2DT table;
	
	/* embedding energy */
	tmp.Dimension(np);
	table.Dimension(np, 2);
	in >> tmp;
	table.SetColumn(1, tmp);
	double p = 0.0;
	for (int i = 0; i < np; i++)
	{
		table(i,0) = p;
		p += dp;
	}
	fEmbeddingEnergy =  new CubicSplineT(table, CubicSplineT::kParabolic);
	if (!fEmbeddingEnergy) throw ExceptionT::kOutOfMemory;
	
	/* phi function */
	tmp.Dimension(nr);
	table.Dimension(nr, 2);
	in >> tmp;
	tmp *= sqrt(27.2*0.529);
	
	table.SetColumn(1, tmp);
	double r = 0.0;
	for (int j = 0; j < nr; j++)
	{
		/* store radius */
		table(j,0) = r;

		/* convert z to phi */
		double z = table(j,1);
		if (j == 0)
			table(j,1) = z*z/dr;
		else
			table(j,1) = z*z/r;
		
		/* increment r */
		r += dr;
	}
	//fPairPotential =  new PhiSplineT(table, CubicSplineT::kFreeRun, r_cut);
	fPairPotential =  new CubicSplineT(table, CubicSplineT::kFreeRun);
	if (!fPairPotential) throw ExceptionT::kOutOfMemory;
	
	/* electron density function */
	in >> tmp;
	table.SetColumn(1, tmp);
	fElectronDensity =  new CubicSplineT(table, CubicSplineT::kParabolic);
	if (!fElectronDensity) throw ExceptionT::kOutOfMemory;
}

/* lattice parameter */
double FBD_EAMGlue::LatticeParameter(void) const
{
	return fLatticeParameter;
}

/**********************************************************************
* Private
**********************************************************************/

/* glue functions initialized from stream in constructor */
void FBD_EAMGlue::SetPairPotential(void) { }
void FBD_EAMGlue::SetEmbeddingEnergy(void) { }
void FBD_EAMGlue::SetElectronDensity(void) { }
