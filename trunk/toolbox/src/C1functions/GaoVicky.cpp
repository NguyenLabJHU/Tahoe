/* $Id: GaoVicky.cpp,v 1.2 2002-07-02 19:56:31 cjkimme Exp $ */
/* created: paklein (12/26/1998)                                          */

#include "GaoVicky.h"
#include <math.h>
#include <iostream.h>
#include "ExceptionCodes.h"
#include "dArrayT.h"

/* constructor */

using namespace Tahoe;

GaoVicky::GaoVicky(double A, double B, double C, double D, double L):
	fA(A),
	fB(B),
	fC(C),
	fD(D),
	fL(L)
{
// should insert some consistency checks on the parameters
}

/* I/O */
void GaoVicky::Print(ostream& out) const
{
	/* parameters */
	out << " Potential parameters:\n";
	out << "      A = " << fA << '\n';
	out << "      B = " << fB << '\n';
	out << "      C = " << fC << '\n';
	out << "      D = " << fD << '\n';
	out << "      L = " << fL << '\n';
}

void GaoVicky::PrintName(ostream& out) const
{
	out << "    Gao-Ji3\n";
}

/* returning values */
double GaoVicky::Function(double x) const
{
	double dl = x - fL;
	
	cout << "\n GaoVicky::Function: only f' and f\" have been implemented\n";
	cout <<   " The function value f is not available in closed form.";
	
	throw eGeneralFail;
	return 0.0;
}

double GaoVicky::DFunction(double x) const
{
	double dl = x - fL;
	double BC = fB/fC;
			
        return fA*dl/(1. + exp((-BC + dl)/fD));
}

double GaoVicky::DDFunction(double x) const
{
	double dl  =  x - fL;
	double BC = fB/fC;
	
	return fA/(1. + exp((-BC + dl)/fD)) - fA*dl*exp((-BC + dl)/fD)/
	         (fD*pow(1. + exp((-BC + dl)/fD), 2));
}

/* returning values in groups - derived classes should define
* their own non-virtual function called within this functon
* which maps in to out w/o requiring a virtual function call
* everytime. Default behavior is just to map the virtual functions
* above */
dArrayT& GaoVicky::MapDFunction(const dArrayT& in, dArrayT& out) const
{
	/* dimension checks */
	if (in.Length() != out.Length()) throw eGeneralFail;

	double* pl  = in.Pointer();
	double* pdU = out.Pointer();
	
	for (int i = 0; i < in.Length(); i++)
	{
		double dl = (*pl++) - fL;
		double BC = fB/fC;
		
		*pdU++ = fA*dl/(1. + exp((-BC + dl)/fD));
	}
	
	return out;
}

dArrayT& GaoVicky::MapDDFunction(const dArrayT& in, dArrayT& out) const
{
	/* dimension checks */
	if (in.Length() != out.Length()) throw eGeneralFail;

	double* pl   = in.Pointer();
	double* pddU = out.Pointer();
	
	for (int i = 0; i < in.Length(); i++)
	{
		double dl = (*pl++) - fL;
	        double BC = fB/fC;
		
		*pddU++ = fA/(1. + exp((-BC + dl)/fD)) - fA*dl*exp((-BC + dl)/fD)/
	         (fD*pow(1. + exp((-BC + dl)/fD), 2));
	}
	
	return out;
}
