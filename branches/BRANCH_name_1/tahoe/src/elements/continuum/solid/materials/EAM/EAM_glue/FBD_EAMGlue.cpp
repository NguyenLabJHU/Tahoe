/* $Id: FBD_EAMGlue.cpp,v 1.1.1.1.10.1 2002-06-27 18:03:08 cjkimme Exp $ */
/* created: paklein (01/30/2000)                                          */
/* FBD_EAMGlue.cpp                                                        */

#include "FBD_EAMGlue.h"

#include "fstreamT.h"
#include "PhiSplineT.h"

/* Constructor */

using namespace Tahoe;

FBD_EAMGlue::FBD_EAMGlue(CBLatticeT& lattice, ifstreamT& in):
	EAM(lattice)
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
	 r_cut < 0.0) throw eBadInputValue;
	
	/* work space */
	dArrayT tmp;
	dArray2DT table;
	
	/* embedding energy */
	tmp.Allocate(np);
	table.Allocate(np, 2);
	in >> tmp;
	table.SetColumn(1, tmp);
	double p = 0.0;
	for (int i = 0; i < np; i++)
	{
		table(i,0) = p;
		p += dp;
	}
	fEmbeddingEnergy =  new CubicSplineT(table, CubicSplineT::kParabolic);
	if (!fEmbeddingEnergy) throw eOutOfMemory;
	
	/* phi function */
	tmp.Allocate(nr);
	table.Allocate(nr, 2);
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
	if (!fPairPotential) throw eOutOfMemory;
	
	/* electron density function */
	in >> tmp;
	table.SetColumn(1, tmp);
	fElectronDensity =  new CubicSplineT(table, CubicSplineT::kParabolic);
	if (!fElectronDensity) throw eOutOfMemory;
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
