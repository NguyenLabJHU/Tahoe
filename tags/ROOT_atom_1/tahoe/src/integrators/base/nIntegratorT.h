/* $Id: nIntegratorT.h,v 1.6 2002-07-05 22:27:53 paklein Exp $ */
/* created: paklein (10/14/1996) */

#ifndef _N_CONTROLLERT_H_
#define _N_CONTROLLERT_H_

/* base class */
#include "IntegratorT.h"

/* direct members */
#include "KBC_CardT.h"
#include "ArrayT.h"

namespace Tahoe {

/* forward declarations */
class BasicFieldT;
class dArray2DT;
class iArray2DT;
class dArrayT;
class iArrayT;

/** defines the interface for time integrators of field data. All
 * field arrays must be set with nIntegratorT::RegisterField
 * before calling predictors, correctors, or boundary condition
 * calculations. Note that the predictor is applied to all
 * degrees of freedom, while the correctors are only applied to
 * the \a active degrees of freedom */
class nIntegratorT: virtual public IntegratorT
{
public:

	/** constructor */
	nIntegratorT(void);

	/** destructor */
	virtual ~nIntegratorT(void);

	/** pseudo-boundary conditions for external nodes. For parallel
	 * calculations, external nodes, or "ghost" nodes, are assigned
	 * a boundary condition state in order to remove their associated
	 * degrees of freeom from the local "active" set. */
	virtual KBC_CardT::CodeT ExternalNodeCondition(void) const = 0;

	/** predictor. Maps ALL degrees of freedom forward. */
	virtual void Predictor(BasicFieldT& field) = 0;

	/** corrector. Maps only the ACTIVE degrees of freedom forward.
	 * \param eqnos equations for the degrees of freedom of every node
	 * \param update vector of updates to the active degrees of freedom 
	 * \param eq_start lowest equation number to consider \a active
	 * \param num_eq number of \a active equations beginning with eq_start */
	virtual void Corrector(BasicFieldT& field, const dArrayT& update, int eq_start, 
		int num_eq) = 0;

	/** apply corrector to active equations with a node number map.
	 * \param map list of nodes corresponding to the rows of eqnos and update 
	 * \param flags flags marking the active equations of every node in \e map.
	 *        flags > 0 are considered active.
	 * \param update updates to the nodal field data */
	virtual void MappedCorrector(BasicFieldT& field, const iArrayT& map, 
		const iArray2DT& flags, const dArray2DT& update) = 0;

	/** return the field array needed by nIntegratorT::MappedCorrector. */
	virtual const dArray2DT& MappedCorrectorField(BasicFieldT& field) const = 0;
	
	/** prescribe the field and derivatives consistent BC's */
	virtual void ConsistentKBC(BasicFieldT& field, const KBC_CardT& KBC) = 0;

protected:  	
	
	/** recalculate time stepping constants */
	virtual void nComputeParameters(void) = 0;
};

} // namespace Tahoe 
#endif /* _N_CONTROLLERT_H_ */
