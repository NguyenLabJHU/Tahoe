/* $Id: nExplicitCD.h,v 1.5 2002-07-02 19:55:08 cjkimme Exp $ */
/* created: paklein (03/23/1997) */

#ifndef _N_EXP_CD_H_
#define _N_EXP_CD_H_

/* base class */
#include "ExplicitCD.h"
#include "nControllerT.h"

/** Node controller for an explicit 2nd order accurate, central 
 * difference time-stepping algorithm. */

namespace Tahoe {

class nExplicitCD: public virtual ExplicitCD, public nControllerT
{
public:

	/** constructor */
	nExplicitCD(void);

	/** consistent BC's */
	virtual void ConsistentKBC(BasicFieldT& field, const KBC_CardT& KBC);

	/** pseudo-boundary conditions for external nodes */
	virtual KBC_CardT::CodeT ExternalNodeCondition(void) const;

	/** predictor. Maps ALL degrees of freedom forward. */
	virtual void Predictor(BasicFieldT& field);

	/** corrector - map ACTIVE. See nControllerT::Corrector for more
	 * documentation */
	virtual void Corrector(BasicFieldT& field, const dArrayT& update, 
		int eq_start, int num_eq);

	/** corrector with node number map - map ACTIVE. See 
	 * nControllerT::MappedCorrector for more documentation */
	virtual void MappedCorrector(BasicFieldT& field, const iArrayT& map, 
		const iArray2DT& flags, const dArray2DT& update);

	/** return the field array needed by nControllerT::MappedCorrector. */
	virtual const dArray2DT& MappedCorrectorField(BasicFieldT& field) const;

protected:  	
	
	/** recalculate time stepping constants */
	virtual void nComputeParameters(void);
	
private:

	/** \name predictor constants*/
	/*@{*/
	double dpred_v;
	double dpred_a;
	double vpred_a;
	/*@}*/
	
	/** corrector constant, also used for consistent BC*/  	
	double vcorr_a;
	  	  	
};

} // namespace Tahoe 
#endif /* _N_EXP_CD_H_ */
