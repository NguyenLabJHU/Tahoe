/* $Id: VIB_E_MatT.cpp,v 1.2 2002-07-02 19:55:55 cjkimme Exp $ */
/* created: paklein (11/08/1997)                                          */
/* Base class for isotropic VIB_E_MatT materials.                         */

#include "VIB_E_MatT.h"

#include <math.h>
#include <iostream.h>

#include "Constants.h"
#include "ExceptionCodes.h"
#include "dSymMatrixT.h"
#include "C1FunctionT.h"

/* constructors */

using namespace Tahoe;

VIB_E_MatT::VIB_E_MatT(ifstreamT& in, int nsd):
	VIB(in, nsd, dSymMatrixT::NumValues(nsd), (nsd == 2) ? 5 : 15)
{

}

/* print parameters */
void VIB_E_MatT::PrintName(ostream& out) const
{
	/* inherited */
	VIB::PrintName(out);

	out << "    Lagrangian\n";
}

/*************************************************************************
* Protected
*************************************************************************/

/* returns the strain energy density for the specified strain */
double VIB_E_MatT::VIBEnergyDensity(const dSymMatrixT& E)
{
	/* fill length table */
	ComputeLengths(E);

	/* potential table */
	fPotential->MapFunction(fLengths,fU);
	
	double  energy = 0.0;
	
	double* pU = fU.Pointer();
	double* pj = fjacobian.Pointer();
	
	for (int i = 0; i < fLengths.Length(); i++)
		energy += (*pU++)*(*pj++);
	
	return( energy );
}

/* compute strained lengths */
void VIB_E_MatT::ComputeLengths(const dSymMatrixT& strain)
{
	if (strain.Rows() == 2)
	{
		/* references to the strain components */
		double& E11 = strain[0];
		double& E22 = strain[1];
		double& E12 = strain[2];
	
		/* initialize kernel pointers */
		double* pl  = fLengths.Pointer();

		/* set pointers */
		double *p11, *p22, *p12;
		SetStressPointers2D(p11,p22,p12);
	
		for (int i = 0; i < fLengths.Length(); i++)
			*pl++ = sqrt(1.0 + 2.0*((*p11++)*E11 +
	                                2.0*(*p12++)*E12 +
	                                (*p22++)*E22));
	}
	else
	{
		/* references to the strain components */
		double& E11 = strain[0];
		double& E22 = strain[1];
		double& E33 = strain[2];
		double& E23 = strain[3];
		double& E13 = strain[4];
		double& E12 = strain[5];
	
		/* initialize kernel pointers */
		double* pl  = fLengths.Pointer();
	
		/* set pointers */
		double *p11, *p22, *p33, *p23, *p13, *p12;
		SetStressPointers3D(p11,p22,p33,p23,p13,p12);

		for (int i = 0; i < fLengths.Length(); i++)
			*pl++ = sqrt(1.0 + 2.0*(E11*(*p11++) + E22*(*p22++) + E33*(*p33++)) +
		                 4.0*(E23*(*p23++) + E13*(*p13++) + E12*(*p12++)));
	}
}

/* convenience */
void VIB_E_MatT::SetStressPointers2D(double*& p11,double*& p22,double*& p12)
{
	if (fStressTable.MajorDim() != 3) throw eGeneralFail;	

	/* pointers to the rows */
	p11 = fStressTable(0);
	p22 = fStressTable(1);
	p12 = fStressTable(2);
}
	
void VIB_E_MatT::SetStressPointers3D(double*& p11,double*& p22,double*& p33,
double*& p23,double*& p13,double*& p12)
{
	if (fStressTable.MajorDim() != 6) throw eGeneralFail;	

	/* pointers to the rows */
	p11 = fStressTable(0);
	p22 = fStressTable(1);
	p33 = fStressTable(2);
	p23 = fStressTable(3);
	p13 = fStressTable(4);
	p12 = fStressTable(5);
}

void VIB_E_MatT::SetModuliPointers2D(double*& p11, double*& p22, double*& p26,
							  double*& p16, double*& p12)
{
	if (fModuliTable.MajorDim() != 5) throw eGeneralFail;	

	/* pointers to the rows */
	p11 = fModuliTable(0);
	p22 = fModuliTable(1);
	p26 = fModuliTable(2);
	p16 = fModuliTable(3);
	p12 = fModuliTable(4);
}							

void VIB_E_MatT::SetModuliPointers3D(double*& p11, double*& p12, double*& p13, double*& p14, double*& p15,
double*& p16, double*& p22, double*& p23, double*& p24, double*& p25,
double*& p26, double*& p33, double*& p34, double*& p35, double*& p36)
{
	if (fModuliTable.MajorDim() != 15) throw eGeneralFail;	

	/* pointers to the rows */
	p11 = fModuliTable(0);
	p12 = fModuliTable(1);
	p13 = fModuliTable(2);
	p14 = fModuliTable(3);
	p15 = fModuliTable(4);
	p16 = fModuliTable(5);

	p22 = fModuliTable(6);
	p23 = fModuliTable(7);
	p24 = fModuliTable(8);
	p25 = fModuliTable(9);
	p26 = fModuliTable(10);

	p33 = fModuliTable(11);
	p34 = fModuliTable(12);
	p35 = fModuliTable(13);
	p36 = fModuliTable(14);
}
