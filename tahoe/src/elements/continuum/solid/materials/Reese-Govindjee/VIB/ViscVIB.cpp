/* $Id: ViscVIB.cpp,v 1.1.2.1 2002-10-17 04:38:01 paklein Exp $ */
/* created: TDN (1/19/2000) */

#include <math.h>
#include <iostream.h>

#include "ViscVIB.h"
#include "toolboxConstants.h"
#include "ExceptionT.h"

#include "fstreamT.h"

/* potential functions */
#include "SmithFerrante.h"
#include "SF2.h"
#include "VariViscT.h"
#include "ConstantT.h"
#include "BiQuadraticT.h"

using namespace Tahoe;

//see 	static int NumValues(int nsd) in dSymMatrixT.h

ViscVIB::ViscVIB(ifstreamT& in, int nsd, int numstress, int nummoduli):
	fNumSD(nsd),
	fNumStress(numstress),
	fNumModuli(nummoduli)
{
	/*set potential function*/
	int potentialcode;
	in >> potentialcode;	
	switch(potentialcode)
	{
		case C1FunctionT::kSmithFerrante:
		{
			double AE, BE, AIN, BIN;
			in >> AE >> BE;		
			fPotential_E = new SmithFerrante(AE,BE);
			in >> AIN >> BIN;
			fPotential_I = new SmithFerrante(AIN,BIN);
			break;
		}
		case C1FunctionT::kSF2:
		{
			double AE, BE, AIN, BIN;
			in >> AE >> BE;		
			fPotential_E = new SF2(AE,BE);
			in >> AIN >> BIN;
			fPotential_I = new SF2(AIN,BIN);
			break;
		}
		default:
		
			throw ExceptionT::kBadInputValue;	
	}
	in >> potentialcode;	
	switch(potentialcode)
	{
		case ViscFuncT::kConstant:
		{
			double A;
			in >> A;		
			fShearVisc = new ConstantT(A);
			in >> A;		
			fBulkVisc = new ConstantT(A);
			break;
		}	
		case ViscFuncT::kVariVisc:
		{
		        double n0, d0,zed,Jcrit;
			in >> n0 >> d0 >> zed >> Jcrit;
			fShearVisc = new VariViscT(n0, d0, zed, Jcrit);
			in >> n0 >> d0 >> zed >> Jcrit;
			fBulkVisc = new VariViscT(n0, d0, zed, Jcrit);
			break;
		}
		case ViscFuncT::kBiQuadratic:
		{
		        double A1,A2,B,C;
			in >> A1 >> A2 >> B >> C;
			fShearVisc = new BiQuadraticT(A1, A2, B, C);
			in >> A1 >> A2 >> B >> C;
			fBulkVisc = new BiQuadraticT(A1, A2, B, C);
			break;
		}
        	default:
		
			throw ExceptionT::kBadInputValue;	
	}
	
	/*set viscosity function*/
	if (!fPotential_E || !fPotential_I || !fShearVisc || !fBulkVisc) 
		throw ExceptionT::kOutOfMemory;
}

ViscVIB::~ViscVIB(void)
{
	delete fPotential_E;
	delete fPotential_I;
	delete fShearVisc;
	delete fBulkVisc;
}
/* print parameters */
void ViscVIB::Print(ostream& out) const
{
	out << " Number of spatial dimensions. . . . . . . . . . = " << fNumSD << '\n';
	
	/* potential parameters */
	out << " Elastic potential";
	fPotential_E->Print(out);

	out << " Inelastic potential";
	fPotential_I->Print(out);

	out << " Shear viscosity function";
	fShearVisc->Print(out);

	out << " Bulk Viscosity function ";
	fBulkVisc->Print(out);

}

void ViscVIB::PrintName(ostream& out) const
{
	out << "    Viscoelastic Virtual Internal Bond\n";
	
	/* potential name */
	out << "    Elastic potential:           ";
	fPotential_E -> PrintName(out);

	out << "    Inelastic potential:         ";
	fPotential_I -> PrintName(out);
	
	out << "    Shear viscosity function:    ";
	fShearVisc -> PrintName(out);

	out << "     Bulk viscosity function:    ";
	fBulkVisc -> PrintName(out);
}


/*************************************************************************
 * Protected
 *************************************************************************/

/* allocate memory for all the tables */
void ViscVIB::Allocate(int numbonds)
{
  	/* length table */
	fLengths_E.Allocate(numbonds);
	fLengths_I.Allocate(numbonds);

	/* potential tables */
	fU_E.Allocate(numbonds);
	fdU_E.Allocate(numbonds);
	fddU_E.Allocate(numbonds);

	fU_I.Allocate(numbonds);
	fdU_I.Allocate(numbonds);
	fddU_I.Allocate(numbonds);

  	/* jacobian table */
	fjacobian.Allocate(numbonds);
  
  	/* STRESS angle tables - by associated stress component */
	fStressTable.Allocate(fNumStress, numbonds);
  	  	
  	/* MODULI angle tables - using Cauchy symmetry */ 	
	fModuliTable.Allocate(fNumModuli, numbonds);	
} 
