/* $Id: nStaticIntegrator.h,v 1.6.10.1 2002-12-18 09:39:04 paklein Exp $ */
/* created: paklein (10/14/1996) */
#ifndef _N_STATIC_CONTROLLER_H_
#define _N_STATIC_CONTROLLER_H_

/* base classes */
#include "StaticT.h"
#include "nIntegratorT.h"

namespace Tahoe {

/** nodal integrator for quasistatic systems */
class nStaticIntegrator: public virtual StaticT, public nIntegratorT
{
public:

	/** constructor */
	nStaticIntegrator(void);

	/** consistent BC's */
	virtual void ConsistentKBC(BasicFieldT& field, const KBC_CardT& KBC);

	/** pseudo-boundary conditions for external nodes */
	virtual KBC_CardT::CodeT ExternalNodeCondition(void) const;

	/** predictor. Maps ALL degrees of freedom forward. */
	virtual void Predictor(BasicFieldT& field);

	/** corrector. Maps ALL degrees of freedom forward. */
	virtual void Corrector(BasicFieldT& field, const dArray2DT& update);

	/** corrector - map ACTIVE. See nIntegratorT::Corrector for more
	 * documentation */
	virtual void Corrector(BasicFieldT& field, const dArrayT& update, 
		int eq_start, int num_eq);

	/** corrector with node number map - map ACTIVE. See 
	 * nIntegratorT::MappedCorrector for more documentation */
	virtual void MappedCorrector(BasicFieldT& field, const iArrayT& map, 
		const iArray2DT& flags, const dArray2DT& update);

	/** return the field array needed by nIntegratorT::MappedCorrector. */
	virtual const dArray2DT& MappedCorrectorField(BasicFieldT& field) const;
	  	
protected:  	
	
	/** recalculate time stepping constants */
	virtual void nComputeParameters(void);	
};

} // namespace Tahoe 
#endif /* _N_STATIC_CONTROLLER_H_ */