/* $Id: IntegratorT_factory.cpp,v 1.1.4.1 2004-04-08 07:33:39 paklein Exp $ */
#include "IntegratorT.h"

/* integrators */
#include "StaticIntegrator.h"
#include "LinearStaticIntegrator.h"
#include "TrapezoidIntegrator.h"
#include "LinearHHTalpha.h"
#include "NLHHTalpha.h"
#include "ExplicitCDIntegrator.h"
#include "VerletIntegrator.h"
#include "Gear6Integrator.h"

using namespace Tahoe;

/* return a pointer to a integrator of the specified type */
IntegratorT* IntegratorT::New(int type, bool exception_on_fail)
{
	const char caller[] = "IntegratorT::New";
	IntegratorT* integrator = NULL;
	try {
	switch (type)
	{
		case kLinearStatic:
		{
			integrator = new LinearStaticIntegrator;
			break;		
		}
		case kStatic:
		{
			integrator = new StaticIntegrator;
			break;		
		}
		case kTrapezoid:
		{
			integrator = new TrapezoidIntegrator;
			break;		
		}
		case kLinearHHT:
		{
			integrator = new LinearHHTalpha(0.0);
			break;				
		}
		case kNonlinearHHT:
		{
			integrator = new NLHHTalpha(0.0);
			break;
		}
		case kExplicitCD:
		{
			integrator = new ExplicitCDIntegrator;
			break;		
		}
		case kVerlet:
		{
			integrator = new VerletIntegrator;
			break;
		}
		case kGear6:
		{
			integrator = new Gear6Integrator;
			break;
		}
		default:
			if (exception_on_fail)
				ExceptionT::GeneralFail(caller, "unrecognized type %d", type);
	} }

#ifdef __NEW_THROWS__
	catch (bad_alloc) { integrator = NULL; }
#else
	catch (ExceptionT::CodeT) { integrator = NULL; }
#endif	
	
	/* fail */
	if (!integrator) ExceptionT::GeneralFail(caller);

	return integrator;
}
