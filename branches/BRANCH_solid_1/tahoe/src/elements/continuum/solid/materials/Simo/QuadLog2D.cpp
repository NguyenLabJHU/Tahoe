/* $Id: QuadLog2D.cpp,v 1.1.1.1.2.2 2001-06-22 14:18:08 paklein Exp $ */
/* created: paklein (06/28/1997)                                          */
/* (2D <-> 3D) translator for the QuadLog3D.                              */

#include "QuadLog2D.h"
#include <math.h>
#include <iostream.h>

/* constructor */
QuadLog2D::QuadLog2D(ifstreamT& in, const FiniteStrainT& element):
	QuadLog3D(in, element),
	Material2DT(in, kPlaneStrain),
	fb_2D(2),
	fStress2D(2),
	fModulus2D(dSymMatrixT::NumValues(2))
{
	fDensity *= fThickness;
}

/* modulus */
const dMatrixT& QuadLog2D::c_ijkl(void)
{
	/* deformation */
	Compute_b(fb_2D);

	/* Compute plane strain stretch */
	fb.ExpandFrom2D(fb_2D);
	fb(2,2) = 1.0; /* plane strain */
	
	/* 3D calculation */
	ComputeModuli(fb, fModulus);

	/* 3D -> 2D */
	fModulus2D.Rank4ReduceFrom3D(fModulus);
	fModulus2D *= fThickness;

	return fModulus2D;
}
	
/* stresses */
const dSymMatrixT& QuadLog2D::s_ij(void)
{
	/* deformation */
	Compute_b(fb_2D);

	/* Compute plane strain stretch */
	fb.ExpandFrom2D(fb_2D);
	fb(2,2) = 1.0; /* plane strain */
	
	/* 3D calculation */
	ComputeCauchy(fb, fStress);

	/* 3D -> 2D */
	fStress2D.ReduceFrom3D(fStress);
	fStress2D *= fThickness;

	return fStress2D;
}

/* strain energy density */
double QuadLog2D::StrainEnergyDensity(void)
{
	/* deformation */
	Compute_b(fb_2D);

	/* principal values */
	fb_2D.PrincipalValues(fEigs);
	fEigs[2] = 1.0; /* plane strain */
	
	/* logarithmic stretches */
	LogStretches(fEigs);

	return fThickness*ComputeEnergy(floge);
}

/* print parameters */
void QuadLog2D::Print(ostream& out) const
{
	/* inherited */
	QuadLog3D::Print(out);
	Material2DT::Print(out);
}

/*************************************************************************
* Protected
*************************************************************************/

/* print name */
void QuadLog2D::PrintName(ostream& out) const
{
	/* inherited */
	QuadLog3D::PrintName(out);

	out << "    Plane Strain\n";
}
