/* $Id: IntegratorT.cpp,v 1.2.2.1 2002-04-24 01:29:21 paklein Exp $ */
/* created: paklein (10/14/1996) */

#include "IntegratorT.h"

#include <iostream.h>

#include "dArrayT.h"
//#include "NodeManagerT.h"

/* constructor */
IntegratorT::IntegratorT(void): fdt(-1.0) { }

/* destructor */
IntegratorT::~IntegratorT(void) { }

/* set the time step size */
void IntegratorT::SetTimeStep(double timestep)
{
	/* check */
	if (timestep <= 0.0) throw eGeneralFail;
	
	/* re-calculate time-stepping parameters */
	fdt = timestep;
	ComputeParameters();
}

/* take control of the time at which the external force vector
* is formed.  Default action to simply call the NodeManagerT's
* FormRHS function */
void IntegratorT::FormNodalForce(NodeManagerT* nodeboss) const
{
#pragma unused(nodeboss)
#pragma message("IntegratorT::FormNodalForce: need this????")
//	nodeboss->FormRHS();
}
