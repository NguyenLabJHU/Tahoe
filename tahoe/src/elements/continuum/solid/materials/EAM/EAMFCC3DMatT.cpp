/* $Id: EAMFCC3DMatT.cpp,v 1.5.2.1 2002-10-28 06:48:48 paklein Exp $ */
/* created: paklein (10/25/1998) */
#include "EAMFCC3DMatT.h"

#include <math.h>
#include <iostream.h>

#include "toolboxConstants.h"

#include "fstreamT.h"
#include "EAMFCC3DSym.h"
#include "dMatrixT.h"

using namespace Tahoe;

/* material parameters */
const int knsd = 3;

const double sqrt2 = sqrt(2.0);
const double sqrt3 = sqrt(3.0);

/* constructor */
EAMFCC3DMatT::EAMFCC3DMatT(ifstreamT& in, const FDMatSupportT& support):
	NL_E_MatT(in, support),
	fEAM(NULL)
{
	/* read parameters */
	in >> fOrientCode;
	in >> fEAMCode;
	
	/* construct EAM solver */
	switch (fOrientCode)
	{
		case kFCC3Dnatural:
		{
			fEAM = new EAMFCC3DSym(in, fEAMCode, knsd);
			break;
		}	
		case kFCC3D110:
		{
			dMatrixT Q(3);
			
			double cos45 = 0.5*sqrt2;
			
			/* transform global xy-plane into [110] */
			Q = 0.0;
			
			Q(0,0) = 1.0;
			Q(1,1) = Q(2,2) = cos45;
			Q(1,2) =-cos45;
			Q(2,1) = cos45;
			
			fEAM = new EAMFCC3DSym(in, Q, fEAMCode, knsd);
			break;
		}	
		case kFCC3D111_a:
		{
			dMatrixT Q(3);
			Q = 0.0;
			
			double rt2b2 = sqrt2/2.0;
			double rt3b3 = sqrt3/3.0;
			double rt6b6 = (sqrt2*sqrt3)/6.0;
			double rt23  = sqrt2/sqrt3;
			
			Q(0,0) =-rt2b2;
			Q(0,1) =-rt6b6;
			Q(0,2) = rt3b3;
			
			Q(1,0) = rt2b2;
			Q(1,1) =-rt6b6;
			Q(1,2) = rt3b3;
			
			Q(2,0) = 0.0;
			Q(2,1) = rt23;
			Q(2,2) = rt3b3;
			
			fEAM = new EAMFCC3DSym(in, Q, fEAMCode, knsd);
			break;
		}
		case kFCC3D111_b:
		{
			dMatrixT Q(3);
			Q = 0.0;
			
			double rt2b2 = sqrt2/2.0;
			double rt3b3 = sqrt3/3.0;
			double rt6b6 = (sqrt2*sqrt3)/6.0;
			double rt23  = sqrt2/sqrt3;
			
			Q(0,0) = rt2b2;
			Q(0,1) = rt6b6;
			Q(0,2) = rt3b3;
			
			Q(1,0) =-rt2b2;
			Q(1,1) = rt6b6;
			Q(1,2) = rt3b3;
			
			Q(2,0) = 0.0;
			Q(2,1) =-rt23;
			Q(2,2) = rt3b3;
			
			fEAM = new EAMFCC3DSym(in, Q, fEAMCode, knsd);
			break;
		}
		case kPrescribed:
		{
			cout << "\n EAMFCC3DMatT::EAMFCC3DMatT: orientation option not implemented: "
			     << kPrescribed << endl;
			break;
		}		
		default:
		{
			cout << "\n EAMFCC3DMatT::EAMFCC3DMatT: unknown orientation code:" << fOrientCode;
			cout << endl;
			throw ExceptionT::kBadInputValue;
		}
	}
	
	if (!fEAM) throw ExceptionT::kOutOfMemory;
	fEAM->Initialize();	
}

/* destructor */
EAMFCC3DMatT::~EAMFCC3DMatT(void) { delete fEAM; }

/* I/O functions */
void EAMFCC3DMatT::Print(ostream& out) const
{
	/* inherited */
	NL_E_MatT::Print(out);

	/* print EAM solver data */
	fEAM->Print(out);
}

/*************************************************************************
* Protected
*************************************************************************/

void EAMFCC3DMatT::PrintName(ostream& out) const
{
	/* inherited */
	NL_E_MatT::PrintName(out);

	const char* planes[] = {"natural", "110", "111"};

	out << "    EAM FCC 3D <" << planes[fOrientCode] << "> orientation\n";
}

/*************************************************************************
* Private
*************************************************************************/

void EAMFCC3DMatT::ComputeModuli(const dSymMatrixT& E, dMatrixT& moduli)
{
	/* EAM solver */
	fEAM->Moduli(moduli, E);
}

/* returns the stress corresponding to the given strain - the strain
* is the reduced index Green strain vector, while the stress is the
* reduced index 2nd Piola-Kirchhoff stress vector */
void EAMFCC3DMatT::ComputePK2(const dSymMatrixT& E, dSymMatrixT& PK2)
{
	/* EAM solver */
	fEAM->SetStress(E, PK2);
}

/* returns the strain energy density for the specified strain */
double EAMFCC3DMatT::ComputeEnergyDensity(const dSymMatrixT& E)
{
	/* EAM solver */
	return fEAM->EnergyDensity(E);
}
