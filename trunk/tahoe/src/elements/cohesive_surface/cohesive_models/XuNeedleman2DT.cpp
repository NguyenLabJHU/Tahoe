/* $Id: XuNeedleman2DT.cpp,v 1.10 2002-07-02 19:55:17 cjkimme Exp $ */
/* created: paklein (11/14/1997) */

#include "XuNeedleman2DT.h"

#include <iostream.h>
#include <math.h>

#include "ExceptionCodes.h"
#include "fstreamT.h"

/* class parameters */

using namespace Tahoe;

const int    knumDOF = 2;
const double kExpMax = 100;

/* constructor */
XuNeedleman2DT::XuNeedleman2DT(ifstreamT& in): SurfacePotentialT(knumDOF)
{
	in >> q; // phi_t/phi_n
	in >> r; // delta_n* /d_n
	if (q < 0.0 || r < 0.0) throw eBadInputValue;
	
	in >> d_n; // characteristic normal opening
	in >> d_t; // characteristic tangent opening
	if (d_n < 0.0 || d_t < 0.0) throw eBadInputValue;
	
	in >> phi_n; // mode I work to fracture
	if (phi_n < 0.0) throw eBadInputValue;

	in >> r_fail; // d/d_(n/t) for which surface is considered failed
	if (r_fail < 1.0) throw eBadInputValue;

	in >> fKratio; // stiffening ratio
	if (fKratio < 0.0) throw eBadInputValue;
	fK = fKratio*phi_n/(d_n*d_n);
}

/* surface potential */
double XuNeedleman2DT::FractureEnergy(const ArrayT<double>& state)
{
#pragma unused(state)
	return phi_n; 
}

double XuNeedleman2DT::Potential(const dArrayT& jump_u, const ArrayT<double>& state)
{
#pragma unused(state)
#if __option(extended_errorcheck)
	if (jump_u.Length() != knumDOF) throw eSizeMismatch;
	if (state.Length() != NumStateVariables()) throw eGeneralFail;
#endif

	double z1, z2, z3, z4, z5, z6, z7, z8;

	double u_t = jump_u[0];
	double u_n = jump_u[1];

	z1 = 1./d_n;
	z2 = 1./(d_t*d_t);
	z3 = -q;
	z4 = -1. + r;
	z5 = -r;
	z6 = u_t*u_t;
	z7 = -u_n*z1;
	z1 = u_n*z1;
	z8 = 1. + z3;
	z3 = r + z3;
	z4 = 1./z4;
	z2 = -z2*z6;
	z2 = exp(z2);
	z6 = exp(z7);
	z3 = z1*z3*z4;
	z1 = 1. + z1 + z5;
	z3 = q + z3;
	z1 = z1*z4*z8;
	z2 = -z2*z3;
	z1 = z1 + z2;
	z1 = phi_n*z1*z6;
	// phi_n + z1

	/* penetration */
	if (u_n < 0.0) z1 += 0.5*u_n*u_n*fK;

	return phi_n + z1;
}
	
/* traction vector given displacement jump vector */	
const dArrayT& XuNeedleman2DT::Traction(const dArrayT& jump_u, ArrayT<double>& state, const dArrayT& sigma)
{
#pragma unused(state)
#pragma unused(sigma)
#if __option(extended_errorcheck)
	if (jump_u.Length() != knumDOF) throw eSizeMismatch;
	if (state.Length() != NumStateVariables()) throw eGeneralFail;
#endif

	double z1, z2, z3, z4, z5, z6, z7, z8, z9, z10, z11;

	double u_t = jump_u[0];
	double u_n = jump_u[1];


	z1 = 1./d_n;
	z2 = 1./(d_t*d_t);
	z3 = -q;
	z4 = -1. + r;
	z5 = -r;
	z6 = u_t*u_t;
	z7 = -u_n*z1;
	z8 = u_n*z1;
	z9 = 1. + z3;
	z3 = r + z3;
	z4 = 1./z4;
	z6 = -z2*z6;
	z10 = exp(z6); //don't limit shear opening
	// limit compressive deformation
	if (z7 > kExpMax)
	{
		cout << "\n XuNeedleman2DT::Traction: exp(x): x = " << z7 << " > kExpMax" << endl;
		throw eBadJacobianDet;
	}
	z11 = exp(z7);
	z6 = z6 + z7; // since (z6 < 0), (z6' < z7) and z7 is checked above
	z7 = z3*z4*z8;
	z5 = 1. + z5 + z8;
	z8 = z1*z4*z9;
	z6 = exp(z6);
	z3 = -z1*z10*z3*z4;
	z7 = q + z7;
	z4 = z4*z5*z9;
	z3 = z3 + z8;
	z5 = -z10*z7;
	z2 = 2.*phi_n*u_t*z2*z6*z7;
	z3 = phi_n*z11*z3;
	z4 = z4 + z5;
	z1 = -phi_n*z1*z11*z4;
	z1 = z1 + z3;
	//z1 = List(z2,z1);

	fTraction[0] = z2;
	fTraction[1] = z1;

	/* penetration */
	if (u_n < 0.0) fTraction[1] += u_n*fK;

	return fTraction;
}

/* potential stiffness */
const dMatrixT& XuNeedleman2DT::Stiffness(const dArrayT& jump_u, const ArrayT<double>& state, const dArrayT& sigma)
{
#pragma unused(state)
#pragma unused(sigma)
#if __option(extended_errorcheck)
	if (jump_u.Length() != knumDOF) throw eSizeMismatch;
	if (state.Length() != NumStateVariables()) throw eGeneralFail;
#endif

	double z1, z2, z3, z4, z5, z6, z7, z8, z9, z10, z11, z12;
	double z13, z14, z15, z16;

	double u_t = jump_u[0];
	double u_n = jump_u[1];

	z1 = 1./(d_n*d_n);
	z2 = 1./d_n;
	z3 = pow(d_t,-4.);
	z4 = 1./(d_t*d_t);
	z5 = -q;
	z6 = -1. + r;
	z7 = -r;
	z8 = u_t*u_t;
	z9 = -u_n*z2;
	z10 = u_n*z2;
	z11 = 1. + z5;
	z5 = r + z5;
	z6 = 1./z6;
	z12 = -z4*z8;
	z13 = exp(z12);
	z14 = exp(z9);
	z15 = z10*z5*z6;
	z16 = z11*z2*z6;
	z7 = 1. + z10 + z7;
	z9 = z12 + z9;
	z9 = exp(z9);
	z10 = q + z15;
	z7 = z11*z6*z7;
	z11 = -z13*z2*z5*z6;
	z12 = -z10*z13;
	z11 = z11 + z16;
	z5 = 2.*phi_n*u_t*z2*z4*z5*z6*z9;
	z6 = 2.*phi_n*z10*z4*z9;
	z4 = -2.*phi_n*u_t*z10*z2*z4*z9;
	z3 = -4.*phi_n*z10*z3*z8*z9;
	z7 = z12 + z7;
	z2 = -2.*phi_n*z11*z14*z2;
	z4 = z4 + z5;
	z3 = z3 + z6;
	z1 = phi_n*z1*z14*z7;

	// {{z3, z4}, {z4, z1 + z2}}

	fStiffness[0] = z3;
	fStiffness[1] = z4;
	fStiffness[2] = z4;
	fStiffness[3] = z1 + z2;

	/* penetration */
	if (u_n < 0.0) fStiffness[3] += fK;

	return fStiffness;
}

/* surface status */
SurfacePotentialT::StatusT XuNeedleman2DT::Status(const dArrayT& jump_u, const ArrayT<double>& state)
{
#pragma unused(state)
#if __option(extended_errorcheck)
	if (state.Length() != NumStateVariables()) throw eGeneralFail;
#endif

	double u_t1 = jump_u[0];
	double u_t  = sqrt(u_t1*u_t1);
	double u_n  = jump_u[1];
	
	/* square box for now */
	if (u_t > r_fail*d_t || u_n > r_fail*d_n)
		return Failed;
	else if (u_t > d_t || u_n > d_n)
		return Critical;
	else
		return Precritical;
}

void XuNeedleman2DT::PrintName(ostream& out) const
{
	out << "    Xu-Needleman 2D\n";
}

/* print parameters to the output stream */
void XuNeedleman2DT::Print(ostream& out) const
{
	out << " Surface energy ratio (phi_t/phi_n). . . . . . . = " << q       << '\n';
	out << " Critical opening ratio (delta_n* /d_n). . . . . = " << r       << '\n';
	out << " Characteristic normal opening to failure. . . . = " << d_n     << '\n';
	out << " Characteristic tangential opening to failure. . = " << d_t     << '\n';
	out << " Mode I work to fracture (phi_n) . . . . . . . . = " << phi_n   << '\n';
	out << " Failure ratio (d_n/delta_n or d_t/delta_t). . . = " << r_fail   << '\n';
	out << " Penetration stiffness multiplier. . . . . . . . = " << fKratio << '\n';
}
