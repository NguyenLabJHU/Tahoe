/* $Id: SWDiamond100.cpp,v 1.5.30.1 2004-01-21 19:10:02 paklein Exp $ */
/* created: paklein (08/25/1996) */
#include "SWDiamond100.h"
#include <math.h>
#include <iostream.h>

using namespace Tahoe;

/* constructor */
SWDiamond100::SWDiamond100(ifstreamT& in, const FSMatSupportT& support):
	ParameterInterfaceT("Stillinger_Weber_DC_100"),
	SWMaterial2D(in, support)
{

}

/*************************************************************************
* Protected
*************************************************************************/

/*
* Print name.
*/
void SWDiamond100::PrintName(ostream& out) const
{
	/* inherited */
	SWMaterial2D::PrintName(out);

	out << "    SW Diamond <100> 2D\n";
}

/*
* Compute the symetric Cij reduced index matrix.
*/
void SWDiamond100::ComputeModuli(const dSymMatrixT& E, dMatrixT& moduli)
{
	double z1, z2, z3, z4, z5, z6, z7, z8, z9, z10, z11, z12, z13, z14, z15;
	double z16, z17, z18, z19, z20, z21, z22, z23, z24, z25,z26, z27, z28;
	double z29, z30, z31, z32, z33, z34, z35, z36, z37, z38, z39, z40;
	double z41, z42, z43, z44, z45, z46, z47, z48, z49, z50, z51, z52, z53;
	double z54, z55, z56, z57, z58, z59, z60, z61, z62, z63, z64, z65, z66;
	double z67, z68, z69, z70, z71, z72, z73, z74, z75, z76, z77, z78, z79;
	double z80, z81, z82, z83, z84, z85, z86, z87, z88, z89, z90, z91, z92;
	double z93, z94, z95, z96, z97, z98, z99, z100, z101, z102, z103, z104;
	
	double E11 = E[0];
	double E22 = E[1];
	double E12 = E[2];

	double a = 1.0 + ThermalElongation();
		
	z1 = pow(2.,-2./3);
	z2 = pow(2.,1./3);
	z3 = pow(2.,2./3);
	z4 = 1./sqrt(3.0);
	z5 = sqrt(3.0);
	z6 = pow(a,-2);
	z7 = 1./a;
	z8 = pow(a,3);
	z9 = pow(a,4);
	z10 = pow(fdelta,2);
	z11 = -2*E11;
	z12 = 2*E11;
	z13 = -4*E12;
	z14 = 4*E12;
	z15 = -2*E22;
	z16 = 2*E22;
	z17 = pow(fgamma,2);
	z18 = -frcut;
	z15 = -1 + z15;
	z19 = -1 + z11 + z16;
	z16 = 3 + z12 + z16;
	z12 = z12 + z15;
	z11 = z11 + z15;
	z15 = z13 + z16;
	z16 = z14 + z16;
	z13 = z11 + z13;
	z11 = z11 + z14;
	z14 = pow(z15,-4);
	z20 = pow(z15,-3);
	z21 = pow(z15,-2);
	z22 = 1./z15;
	z15 = z15*z2;
	z23 = pow(z16,-4);
	z24 = pow(z16,-3);
	z25 = pow(z16,-2);
	z26 = 1./z16;
	z16 = z16*z2;
	z27 = -8*z11*z20;
	z28 = z11*z20;
	z29 = -8*z21;
	z30 = 8*z21;
	z31 = -2*z11*z21;
	z32 = z11*z21;
	z33 = -2*z22;
	z34 = 2*z22;
	z11 = z11*z22;
	z35 = pow(z15,-5./2);
	z36 = pow(z15,-3./2);
	z37 = 1./sqrt(z15);
	z15 = sqrt(z15);
	z38 = 8*z13*z24;
	z39 = 8*z25;
	z40 = -2*z13*z25;
	z41 = -2*z26;
	z13 = z13*z26;
	z42 = pow(z16,-5./2);
	z43 = pow(z16,-3./2);
	z44 = 1./sqrt(z16);
	z16 = sqrt(z16);
	z28 = 8*z28;
	z32 = 2*z32;
	z1 = 9*fB*z1*z9;
	z21 = z1*z21;
	z1 = z1*z25;
	z25 = z27 + z29;
	z27 = z31 + z33;
	z11 = 1./3 + z11;
	z15 = z15*z4;
	z29 = z38 + z39;
	z31 = z40 + z41;
	z13 = 1./3 + z13;
	z33 = 6*z37*z42;
	z38 = -4*z36*z43;
	z39 = z36*z43;
	z40 = -4*z3*z37*z43;
	z41 = z3*z37*z43;
	z42 = -6*z35*z44;
	z35 = z35*z44;
	z45 = -4*z3*z36*z44;
	z46 = z3*z36*z44;
	z47 = -2*z2*z37*z44;
	z48 = z2*z37*z44;
	z16 = z16*z4;
	z28 = z28 + z30;
	z30 = z32 + z34;
	z21 = -1 + z21;
	z1 = -1 + z1;
	z32 = z15*z7;
	z34 = z19*z33;
	z33 = z12*z33;
	z49 = z19*z38;
	z38 = z12*z38;
	z50 = z19*z39;
	z39 = 4*z12*z39;
	z51 = 4*z41;
	z41 = -z41;
	z52 = z19*z42;
	z42 = z12*z42;
	z53 = z19*z35;
	z35 = 6*z12*z35;
	z54 = 4*z46;
	z55 = -z46;
	z56 = z19*z55;
	z55 = z12*z55;
	z57 = z19*z46;
	z46 = z12*z46;
	z58 = 2*z48;
	z59 = z19*z48;
	z48 = z12*z48;
	z60 = z16*z7;
	z50 = 4*z50;
	z19 = z19*z41;
	z12 = z12*z41;
	z41 = 6*z53;
	z53 = 1./3 + z59;
	z48 = 1./3 + z48;
	z59 = z19 + z58;
	z58 = z12 + z58;
	z61 = z56 + z59;
	z59 = z57 + z59;
	z62 = z55 + z58;
	z58 = z46 + z58;
	z60 = z18 + z60;
	z63 = pow(z27,2);
	z64 = pow(z11,2);
	z15 = z15 + z18;
	z65 = pow(z31,2);
	z66 = pow(z13,2);
	z19 = z19 + z47;
	z56 = z19 + z56;
	z19 = z19 + z57;
	z12 = z12 + z47;
	z47 = z12 + z55;
	z12 = z12 + z46;
	z16 = z16 + z18;
	z46 = pow(z30,2);
	z18 = z18 + z32;
	z32 = z34 + z41;
	z41 = z32 + z50;
	z50 = z40 + z41 + z45;
	z55 = z40 + z54;
	z57 = z32 + z49 + z55;
	z35 = z33 + z35;
	z55 = z35 + z38 + z55;
	z39 = z35 + z39;
	z67 = z39 + z40 + z45;
	z68 = z51 + z54;
	z69 = z41 + z68;
	z68 = z39 + z68;
	z70 = z45 + z51;
	z32 = z32 + z49 + z70;
	z35 = z35 + z38 + z70;
	z34 = z34 + z52;
	z38 = z34 + z54;
	z49 = z34 + z40;
	z52 = z34 + z45;
	z34 = z34 + z51;
	z33 = z33 + z42;
	z42 = z33 + z54;
	z40 = z33 + z40;
	z45 = z33 + z45;
	z33 = z33 + z51;
	z51 = pow(z53,2);
	z54 = pow(z48,2);
	z70 = pow(z61,2);
	z71 = pow(z59,2);
	z72 = pow(z62,2);
	z73 = pow(z58,2);
	z74 = pow(z60,-4);
	z75 = pow(z60,-3);
	z76 = pow(z60,-2);
	z60 = 1./z60;
	z77 = pow(z15,-4);
	z78 = pow(z15,-3);
	z79 = pow(z15,-2);
	z15 = 1./z15;
	z80 = pow(z56,2);
	z81 = pow(z19,2);
	z82 = pow(z47,2);
	z83 = pow(z12,2);
	z84 = pow(z16,-4);
	z85 = pow(z16,-3);
	z86 = pow(z16,-2);
	z16 = 1./z16;
	z87 = pow(z18,-4);
	z88 = pow(z18,-3);
	z89 = pow(z18,-2);
	z18 = 1./z18;
	z60 = fdelta*z60;
	z90 = 2*fgamma;
	z91 = z15 + z16;
	z18 = fdelta*z18;
	z15 = z15*z90;
	z16 = z16*z90;
	z92 = -4*z2*z22*z78/3;
	z93 = -2*z2*z22*z78/3;
	z94 = 2*z2/3;
	z78 = z22*z78;
	z95 = z78*z94;
	z78 = z2*z78;
	z78 = 4*z78/3;
	z85 = z26*z85;
	z96 = z85*z94;
	z85 = 4*z2*z85/3;
	z79 = z4*z79;
	z97 = -2*z3*z36*z79;
	z98 = -z79;
	z36 = z3*z36;
	z99 = z36*z79;
	z100 = z36*z98;
	z101 = 2*z99;
	z102 = z2*z37;
	z79 = z102*z79;
	z98 = z102*z98;
	z86 = z4*z86;
	z102 = z3*z43*z86;
	z103 = 2*z102;
	z104 = -(z2*z44*z86);
	z60 = exp(z60);
	z18 = exp(z18);
	z15 = exp(z15);
	z16 = exp(z16);
	z91 = fgamma*z91;
	z92 = z92 + z97;
	z78 = z101 + z78;
	z96 = z102 + z96;
	z85 = z103 + z85;
	z97 = z104 + z79;
	z98 = z104 + z98;
	z95 = z95 + z96 + z99;
	z93 = z100 + z93 + z96;
	z91 = exp(z91);
	z96 = 4*flambda;
	z90 = flambda*z90;
	z99 = pow(z97,2);
	z100 = pow(z98,2);
	z101 = z15*z96;
	z65 = z16*z65*z96;
	z102 = z15*z64*z90;
	z85 = z16*z66*z85*z90;
	z63 = z101*z63;
	z46 = z101*z46;
	z90 = z102*z92;
	z78 = z102*z78;
	z92 = flambda*z17*z2;
	z102 = -8*z15*z22*z64*z77*z92/3;
	z64 = z15*z22*z64*z77*z92;
	z64 = 8*z64/3;
	z66 = 8*z16*z26*z66*z84*z92/3;
	z25 = z101*z11*z25;
	z77 = -16*fgamma*flambda*z11*z15*z27*z79;
	z84 = fgamma*flambda*z11*z15*z27*z79;
	z84 = 8*z84;
	z13 = z13*z16;
	z16 = z13*z29*z96;
	z13 = -16*fgamma*flambda*z13*z2*z31*z44*z86;
	z5 = fA*fB*fdelta*z3*z5*z8;
	z8 = -24*z18*z20*z37*z5*z89;
	z20 = z18*z20*z37*z5*z89;
	z20 = 24*z20;
	z5 = 24*z24*z44*z5*z60*z76;
	z24 = z101*z11*z28;
	z27 = z101*z27*z30;
	z28 = -8*fgamma*flambda*z11*z15*z30*z79;
	z11 = fgamma*flambda*z11*z15*z30*z79;
	z11 = 16*z11;
	z6 = fA*z6;
	z15 = z10*z6*z94;
	z21 = z18*z21;
	z29 = fA*z21;
	z21 = z21*z22;
	z22 = z21*z6;
	z21 = z15*z21*z87;
	z10 = z10*z22*z87;
	z30 = -4*fdelta*z2*z22*z88/3;
	z22 = fdelta*z2*z22*z88;
	z10 = -2*z10*z2/3;
	z22 = 4*z22/3;
	z31 = -2*fdelta*z29*z36*z4*z7*z89;
	z29 = fdelta*z29*z36*z4*z7*z89;
	z29 = 2*z29;
	z36 = fdelta*z1*z60;
	z6 = 4*z2*z26*z36*z6*z75/3;
	z3 = 2*fA*z3*z36*z4*z43*z7*z76;
	z1 = z1*z26*z60;
	z1 = z1*z15*z74;
	z2 = fA*fB*z2*z9;
	z7 = 216*z2;
	z9 = z14*z18;
	z14 = -216*z2*z9;
	z2 = z2*z9;
	z9 = z7*z9;
	z7 = z23*z60*z7;
	z15 = flambda*z91;
	z18 = z91*z96;
	z1 = z1 + z3 + z5 + z6 + z7;
	z3 = 8*z15;
	z5 = z15*z61;
	z6 = 16*fgamma*z15*z98;
	z7 = z18*z59*z61;
	z15 = z18*z58*z62;
	z23 = z18*z56*z59;
	z26 = z18*z19;
	z36 = z18*z47*z58;
	z37 = z12*z18;
	z43 = z18*z53*z57;
	z44 = z18*z48*z55;
	z32 = z18*z32*z53;
	z35 = z18*z35*z48;
	z55 = z18*z53;
	z57 = z18*z48;
	z60 = z18*z71;
	z71 = z18*z73;
	z73 = z18*z81;
	z74 = z18*z83;
	z51 = z18*z51;
	z75 = z17*z18*z97*z98;
	z18 = z18*z54;
	z8 = z1 + z10 + z14 + z30 + z31 + z8;
	z1 = z1 + z20 + z21 + z22 + z29 + z9;
	z9 = z3*z61;
	z10 = z3*z56;
	z14 = z3*z47*z62;
	z20 = z3*z53;
	z21 = z3*z48;
	z22 = z3*z70;
	z29 = z3*z72;
	z30 = z3*z80;
	z3 = z3*z82;
	z5 = 16*fgamma*z5*z53*z98;
	z31 = z48*z6;
	z6 = z53*z56*z6;
	z61 = z26*z61;
	z70 = z26*z56;
	z26 = fgamma*z26*z53*z98;
	z72 = z37*z62;
	z76 = z37*z47;
	z37 = fgamma*z37*z48*z98;
	z38 = z38*z55;
	z48 = z49*z55;
	z49 = z52*z55;
	z34 = z34*z55;
	z52 = fgamma*z55*z59*z98;
	z42 = z42*z57;
	z40 = z40*z57;
	z45 = z45*z57;
	z33 = z33*z57;
	z55 = fgamma*z57*z58*z98;
	z57 = z17*z51*z97*z98;
	z79 = fgamma*z51;
	z51 = z17*z51;
	z54 = z54*z75;
	z75 = fgamma*z18;
	z17 = z17*z18;
	z18 = z56*z9;
	z53 = fgamma*z53;
	z41 = z20*z41;
	z50 = z20*z50;
	z56 = z20*z69;
	z20 = fgamma*z20*z97;
	z39 = z21*z39;
	z67 = z21*z67;
	z68 = z21*z68;
	z21 = fgamma*z21;
	z69 = z31*z62;
	z31 = z31*z47;
	z80 = z79*z95;
	z79 = z79*z93;
	z81 = z51*z99;
	z51 = z100*z51;
	z82 = z75*z95;
	z75 = z75*z93;
	z83 = z17*z99;
	z17 = z100*z17;
	z86 = z53*z97;
	z87 = z53*z9;
	z53 = z10*z53*z98;
	z59 = z20*z59;
	z19 = z19*z20;
	z20 = z21*z62;
	z62 = z21*z97;
	z21 = z21*z47*z98;
	z9 = z86*z9;
	z10 = z10*z86;
	z86 = z87*z98;
	z87 = z20*z97;
	z20 = z20*z98;
	z58 = z58*z62;
	z47 = z47*z62;
	z12 = z12*z62;
	z13 = z13 + z16 + z65 + z66 + z85;
	z8 = 4*z8;
	z1 = 4*z1;
	z16 = z13 + z24 + z64 + z78 + z80 + z82;
	z13 = z102 + z13 + z25 + z26 + z27 + z28 + z37 + z52 + z54 + z55 + z57 +
		  z75 + z79 + z84 + z90;
	z17 = z16 + z17 + z51 + z63 + z77;
	z11 = z11 + z12 + z16 + z19 + z32 + z35 + z43 + z44 + z46 + z58 + z59 +
		  z60 + z71 + z73 + z74 + z81 + z83;
	z10 = z10 + z13 + z15 + z23 + z34 + z40 + z42 + z49 + z70 + z72 + z87;
	z7 = z13 + z33 + z36 + z38 + z45 + z47 + z48 + z61 + z7 + z76 + z9;
	z6 = z17 + z29 + z30 + z56 + z6 + z67 + z69;
	z3 = z17 + z22 + z3 + z31 + z5 + z50 + z68;
	z5 = z14 + z17 + z18 + z20 + z21 + z39 + z41 + z53 + z86;
	z9 = 3*z11;
	z10 = 3*z10;
	z7 = 3*z7;
	z6 = 3*z6;
	z3 = 3*z3;
	z5 = 3*z5;
	z9 = z1 + z9;
	z10 = z10 + z8;
	z7 = z7 + z8;
	z6 = z1 + z6;
	z3 = z1 + z3;
	z1 = z1 + z5;
	z4 = 64*z4/3;
	z5 = z4*z9;
	z8 = z10*z4;
	z7 = z4*z7;
	z6 = z4*z6;
	z3 = z3*z4;
	z1 = z1*z4;

	// {z6, z3, z5, z7, z8, z1}

	moduli(0,0) = z6;
	moduli(1,1) = z3;
	moduli(2,2) = z5;
	moduli(1,2) = z7;
	moduli(0,2) = z8;
	moduli(0,1) = z1;
	
	/* symmetric */
	moduli.CopySymmetric();	
}

/*
* Compute the symetric 2nd Piola-Kirchhoff reduced index vector.
*/
void SWDiamond100::ComputePK2(const dSymMatrixT& E, dSymMatrixT& PK2)
{
	double z1, z2, z3, z4, z5, z6, z7, z8, z9, z10, z11, z12, z13, z14, z15;
	double z16, z17, z18, z19, z20, z21, z22, z23, z24, z25, z26, z27, z28;
	double z29, z30, z31, z32, z33, z34, z35, z36, z37, z38, z39, z40, z41;

	double	E11 = E[0];
	double	E22 = E[1];
	double	E12 = E[2];

	double a = 1.0 + ThermalElongation();

	z1 = pow(2.,-2./3);
	z2 = pow(2.,1./3);
	z3 = pow(2.,2./3);
	z4 = 1./sqrt(3.0);
	z5 = 1./a;
	z6 = pow(a,4);
	z7 = -2*E11;
	z8 = 2*E11;
	z9 = -4*E12;
	z10 = 4*E12;
	z11 = -2*E22;
	z12 = 2*E22;
	z13 = -frcut;
	z7 = -1 + z7;
	z14 = z10 + z11 + z7;
	z15 = z12 + z7;
	z16 = -1 + z11 + z8;
	z10 = 3 + z10 + z12 + z8;
	z7 = z11 + z7 + z9;
	z8 = 3 + z12 + z8 + z9;
	z9 = pow(z10,-3);
	z11 = pow(z10,-2);
	z12 = 1./z10;
	z10 = z10*z2;
	z17 = pow(z8,-3);
	z18 = pow(z8,-2);
	z19 = 1./z8;
	z8 = z2*z8;
	z20 = -2*z11*z7;
	z21 = -2*z12;
	z7 = z12*z7;
	z12 = pow(z10,-3./2);
	z22 = 1./sqrt(z10);
	z10 = sqrt(z10);
	z23 = -2*z14*z18;
	z24 = z14*z18;
	z25 = -2*z19;
	z26 = 2*z19;
	z14 = z14*z19;
	z19 = pow(z8,-3./2);
	z27 = 1./sqrt(z8);
	z8 = sqrt(z8);
	z24 = 2*z24;
	z1 = 9*fB*z1*z6;
	z11 = z1*z11;
	z1 = z1*z18;
	z18 = z20 + z21;
	z7 = 1./3 + z7;
	z20 = z23 + z25;
	z14 = 1./3 + z14;
	z10 = z10*z4;
	z21 = -z3;
	z3 = z19*z22*z3;
	z23 = -2*z2*z22*z27;
	z25 = z2*z22*z27;
	z8 = z4*z8;
	z24 = z24 + z26;
	z11 = -1 + z11;
	z1 = -1 + z1;
	z26 = z10*z5;
	z28 = z15*z21;
	z29 = z19*z21*z22;
	z21 = z12*z16*z21*z27;
	z30 = z15*z3;
	z3 = z16*z3;
	z31 = 2*z25;
	z15 = z15*z25;
	z25 = z16*z25;
	z32 = z5*z8;
	z19 = z19*z22*z28;
	z12 = z12*z27*z28;
	z16 = z16*z29;
	z28 = pow(z7,2);
	z29 = pow(z14,2);
	z10 = z10 + z13;
	z8 = z13 + z8;
	z26 = z13 + z26;
	z3 = z21 + z3;
	z15 = 1./3 + z15;
	z25 = 1./3 + z25;
	z13 = z13 + z32;
	z30 = z12 + z30;
	z12 = z12 + z19;
	z16 = z16 + z21;
	z19 = z23 + z3;
	z3 = z3 + z31;
	z21 = z23 + z30;
	z30 = z30 + z31;
	z32 = z12 + z23;
	z12 = z12 + z31;
	z23 = z16 + z23;
	z16 = z16 + z31;
	z31 = pow(z10,-2);
	z10 = 1./z10;
	z33 = pow(z8,-2);
	z8 = 1./z8;
	z34 = pow(z26,-2);
	z26 = 1./z26;
	z35 = pow(z15,2);
	z36 = pow(z25,2);
	z37 = pow(z13,-2);
	z13 = 1./z13;
	z13 = fdelta*z13;
	z38 = -(z2*z22*z31*z4);
	z39 = 2*fgamma*z10;
	z40 = -(z2*z27*z33*z4);
	z33 = z2*z27*z33*z4;
	z41 = 2*fgamma*z8;
	z8 = z10 + z8;
	z10 = fdelta*z26;
	z13 = exp(z13);
	z26 = exp(z39);
	z39 = exp(z41);
	z10 = exp(z10);
	z40 = z38 + z40;
	z38 = z33 + z38;
	z8 = fgamma*z8;
	z8 = exp(z8);
	z41 = 4*flambda;
	z7 = z18*z26*z41*z7;
	z14 = z14*z39*z41;
	z18 = z14*z20;
	z14 = z14*z24;
	z20 = fA*z2;
	z13 = z13*z20;
	z24 = -2*fdelta*z1*z13*z27*z37*z4*z5;
	z1 = fdelta*z1*z13*z27*z37*z4*z5;
	z1 = 2*z1;
	z27 = -36*fB*z6;
	z9 = z10*z20*z27*z9;
	z27 = z13*z17*z27;
	z6 = fB*z6;
	z6 = 36*z13*z17*z6;
	z13 = -4*fgamma*flambda;
	z17 = z29*z33*z39;
	z29 = fgamma*z17;
	z17 = z13*z17;
	z29 = z29*z41;
	z22 = z22*z4;
	z2 = z13*z2*z22*z26*z28*z31;
	z5 = -2*fdelta*z10*z11*z20*z22*z34*z5;
	z10 = z41*z8;
	z5 = z5 + z9;
	z9 = fgamma*z10*z36;
	z11 = z24 + z27 + z5;
	z1 = z1 + z5 + z6;
	z5 = z40*z9;
	z6 = z38*z9;
	z9 = z10*z25;
	z13 = z19*z9;
	z3 = z3*z9;
	z9 = z10*z15;
	z19 = z21*z9;
	z9 = z30*z9;
	z8 = 8*flambda*z8;
	z15 = z15*z8;
	z20 = z15*z32;
	z12 = z12*z15;
	z8 = z25*z8;
	z15 = z23*z8;
	z8 = z16*z8;
	z10 = fgamma*z10*z35;
	z16 = z10*z40;
	z10 = z10*z38;
	z2 = z2 + z7;
	z7 = 4*z11;
	z1 = 4*z1;
	z5 = z16 + z17 + z18 + z2 + z5;
	z2 = z10 + z13 + z14 + z19 + z2 + z29 + z3 + z6 + z9;
	z3 = z12 + z15 + z5;
	z5 = z20 + z5 + z8;
	z2 = 3*z2;
	z3 = 3*z3;
	z5 = 3*z5;
	z1 = z1 + z2;
	z2 = z3 + z7;
	z3 = z5 + z7;
	z4 = 64*z4/3;
	z1 = z1*z4;
	z2 = z2*z4;
	z3 = z3*z4;
	
	// {z3, z2, z1}

	PK2[0] = z3;
	PK2[1] = z2;
	PK2[2] = z1;

}

/*
* Returns the strain energy density for the specified strain.
*/
double SWDiamond100::ComputeEnergyDensity(const dSymMatrixT& E)
{
	double z1, z2, z3, z4, z5, z6, z7, z8, z9, z10, z11, z12, z13;
	double z14, z15, z16;

	double E11 = E[0];
	double E22 = E[1];
	double E12 = E[2];

	double a = 1.0 + ThermalElongation();

	z1 = pow(2.,-2./3);
	z2 = pow(2.,1./3);
	z3 = 1./sqrt(3.0);
	z4 = 1./a;
	z5 = pow(a,4);
	z6 = -2*E11;
	z7 = 2*E11;
	z8 = -4*E12;
	z9 = 4*E12;
	z10 = -2*E22;
	z11 = 2*E22;
	z12 = -frcut;
	z6 = -1 + z6;
	z13 = z11 + z6;
	z14 = -1 + z10 + z7;
	z15 = z10 + z6 + z8;
	z8 = 3 + z11 + z7 + z8;
	z6 = z10 + z6 + z9;
	z7 = 3 + z11 + z7 + z9;
	z9 = pow(z8,-2);
	z10 = 1./z8;
	z11 = pow(z7,-2);
	z16 = 1./z7;
	z8 = z2*z8;
	z7 = z2*z7;
	z6 = z10*z6;
	z10 = z15*z16;
	z15 = 1./sqrt(z8);
	z8 = sqrt(z8);
	z16 = 1./sqrt(z7);
	z7 = sqrt(z7);
	z1 = 9*fB*z1*z5;
	z5 = z1*z9;
	z1 = z1*z11;
	z6 = 1./3 + z6;
	z9 = 1./3 + z10;
	z8 = z3*z8;
	z2 = z15*z16*z2;
	z7 = z3*z7;
	z5 = -1 + z5;
	z1 = -1 + z1;
	z10 = z13*z2;
	z2 = z14*z2;
	z11 = z4*z8;
	z4 = z4*z7;
	z6 = pow(z6,2);
	z9 = pow(z9,2);
	z8 = z12 + z8;
	z7 = z12 + z7;
	z10 = 1./3 + z10;
	z2 = 1./3 + z2;
	z11 = z11 + z12;
	z4 = z12 + z4;
	z8 = 1./z8;
	z7 = 1./z7;
	z10 = pow(z10,2);
	z2 = pow(z2,2);
	z11 = 1./z11;
	z4 = 1./z4;
	z12 = 2*fgamma;
	z13 = z7 + z8;
	z11 = fdelta*z11;
	z4 = fdelta*z4;
	z8 = z12*z8;
	z7 = z12*z7;
	z11 = exp(z11);
	z4 = exp(z4);
	z8 = exp(z8);
	z7 = exp(z7);
	z12 = fgamma*z13;
	z13 = 2*fA;
	z12 = exp(z12);
	z14 = 2*flambda;
	z5 = z11*z13*z5;
	z1 = z1*z13*z4;
	z4 = z14*z6*z8;
	z6 = z14*z7*z9;
	z7 = 4*flambda*z12;
	z1 = z1 + z5;
	z5 = z10*z7;
	z2 = z2*z7;
	z1 = 4*z1;
	z2 = z2 + z4 + z5 + z6;
	z2 = 3*z2;
	z1 = z1 + z2;
	z1 = 64*z1*z3/3;

	// z1

	return(z1);
}
