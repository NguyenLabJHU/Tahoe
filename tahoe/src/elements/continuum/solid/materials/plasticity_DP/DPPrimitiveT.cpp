
/* $Id: DPPrimitiveT.cpp,v 1.6 2002-07-02 19:56:09 cjkimme Exp $ */
/* created: myip (06/01/1999)                                             */
/* Base class for Druker-Prager, nonassociative, small strain,        */
/* pressure dependent plasticity model with linear isotropic hardening.*/


#include "DPPrimitiveT.h"

#include <iostream.h>
#include <math.h>

#include "fstreamT.h"
#include "dSymMatrixT.h"


using namespace Tahoe;

const double sqrt23 = sqrt(2.0/3.0);
const double sqrt32 = sqrt(3.0/2.0);

/* constructor */
DPPrimitiveT::DPPrimitiveT(ifstreamT& in): 
        falpha_bar(-1.0),
	ffriction(-1.0),
	fdilation(-1.0),
	fH_prime(0.0),
	fH_delta(1.0)
{
	/* read parameters */
        in >> falpha_bar;  if (falpha_bar < 0.0 ) throw eBadInputValue;
	in >> ffriction;   if (ffriction < 0.0 ) throw eBadInputValue;
	in >> fdilation;
	in >> fH_prime;
	in >> fH_delta;    if (fH_delta > 0.0 ) throw eBadInputValue;
}

/* destructor */
DPPrimitiveT::~DPPrimitiveT(void) { }

/* write parameters */
void DPPrimitiveT::Print(ostream& out) const
{
  out << " Cohesion-like strength parameter. . . . . . = " << falpha_bar << '\n';
  out << " Friction-like parameter . . . . . . . . . . = " << ffriction  << '\n';
  out << " Dilation parameter. . . . . . . . . . . . . = " << fdilation  << '\n';
  out << " Deviatoric hardening parameter. . . . . . . = " << fH_prime   << '\n';
  out << " Localized deviatoric hardening parameter. . = " << fH_delta   << '\n';
}

/***********************************************************************
 * Protected
 ***********************************************************************/

void DPPrimitiveT::PrintName(ostream& out) const
{
  out << "    Nonassociative Drucker-Prager, Pressure-Dependent, Small Strain, \n";
  out << "    Plasticity Model with Linear Isotropic Hardening\n";
}

/*
 * Returns the value of the yield function given the
 * stress vector and state variables, where alpha
 * represents isotropic hardening.
 */
double DPPrimitiveT::YieldCondition(const dSymMatrixT& devstress, 
			const double meanstress, double alpha) const
{
  double kTemp;
  kTemp  = sqrt32*sqrt(devstress.ScalarProduct());
  kTemp += sqrt(3.0)*(-falpha_bar + ffriction*meanstress);
  kTemp += alpha;
  return   kTemp;
}

