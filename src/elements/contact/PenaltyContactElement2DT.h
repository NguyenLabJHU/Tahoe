/* $Id: PenaltyContactElement2DT.h,v 1.6 2002-02-06 22:23:05 rjones Exp $ */

#ifndef _PENALTY_CONTACT_ELEMENT_2D_T_H_
#define _PENALTY_CONTACT_ELEMENT_2D_T_H_

/* base classes */
#include "ContactElementT.h"

class PenaltyContactElement2DT: public ContactElementT
{
  public:

	/* constructor */
	PenaltyContactElement2DT(FEManagerT& fe_manager);
	
	/* initialize */
	virtual void Initialize(void);

	/* writing output */
	virtual void WriteOutput(IOBaseT::OutputModeT mode);

        enum EnforcementParametersT { kPass = 0,
                                kPenalty ,
                                kConsistentTangent ,
                                kAsperityHeightMean,
                                kAsperityHeightStandardDeviation,
                               	kAsperityDensity,
                               	kAsperityTipRadius,
                               	kHertzianModulus, 
				kNumEnfParameters};

	 	
  protected:

	/* print element group data */
	virtual void PrintControlData(ostream& out) const;
		 	
	/* construct the residual force vector, called before LHS */
	virtual void RHSDriver(void);
	
	/* construct the effective mass matrix */
	virtual void LHSDriver(void);
	
	/* total _real_ area of contact for each surface */
	dArrayT fRealArea; 

};

#endif /* _PENALTY_CONTACT_ELEMENT_2D_T_H_ */

