/* $Id: MultiplierContactElement2DT.h,v 1.12 2005-04-14 01:18:53 paklein Exp $ */
// created by : rjones 2001
#ifndef _MULTIPLIER_CONTACT_ELEMENT_2D_T_H_
#define _MULTIPLIER_CONTACT_ELEMENT_2D_T_H_

/* base classes */
#include "ContactElementT.h"

namespace Tahoe {

class MultiplierContactElement2DT: public ContactElementT
{
  public:

	/* constructor */
	MultiplierContactElement2DT(const ElementSupportT& support);

	enum EnforcementParametersT { 
                                kConsistentTangent = 0 ,
                                kPenalty ,
								kGScale,
								kPScale,
								kTolP,
								kMaterialType,
                                kNumEnfParameters};

	enum StatusT {	kNoP = -1,	
					kPZero,
					kPJump,
					kGapZero};

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/	
	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/	

  protected:

	/* set contact status*/
	virtual void SetContactStatus(void);
	
	/* construct the residual force vector, called before LHS */
	virtual void RHSDriver(void);
	
	/* construct the effective mass matrix */
	virtual void LHSDriver(GlobalT::SystemTypeT sys_type);

};

} // namespace Tahoe 
#endif /* _MULTIPLIER_CONTACT_ELEMENT_2D_T_H_ */
