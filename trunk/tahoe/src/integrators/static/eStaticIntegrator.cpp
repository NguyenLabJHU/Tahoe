/* $Id: eStaticIntegrator.cpp,v 1.3 2002-04-02 23:19:24 paklein Exp $ */
/* created: paklein (10/14/1996) */

#include "eStaticIntegrator.h"
#include "Environment.h"
#include "ExceptionCodes.h"

/* constructor */
eStaticIntegrator::eStaticIntegrator(void) { }

/* returns 1 if the algorithm requires M, C, or K and sets const equal
* to the coefficient for the linear combination of components in the
* element effective mass matrix */
int eStaticIntegrator::FormM(double& constM) const
{
#pragma unused(constM)
	return 0;
}

int eStaticIntegrator::FormC(double& constC) const
{
#pragma unused(constC)
	return 0;
}

int eStaticIntegrator::FormK(double& constK) const
{
	constK = 1.0;
	return 1;
}

/* components of the internal force vector */
int eStaticIntegrator::FormMa(double& constMa) const
{
#pragma unused(constMa)
	return 0;
}

int eStaticIntegrator::FormCv(double& constCv) const
{
#pragma unused(constCv)
	return 0;
}

int eStaticIntegrator::FormKd(double& constKd) const
{
	constKd = 1.0;
	return 1;
}

/*************************************************************************
* Protected
*************************************************************************/

/* recalculate constants */
void eStaticIntegrator::eComputeParameters(void) { }
