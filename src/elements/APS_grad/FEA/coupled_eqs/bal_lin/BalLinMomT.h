
//DEVELOPMENT

#ifndef _BALLINMOM_T_H_ 
#define _BALLINMOM_T_H_ 

#include "StringT.h"
#include "Shear_MatlT.h"


namespace Tahoe {

class BalLinMomT
{
public:

	enum Eqn_TypeT { kAPS_Bal_Eq };

	BalLinMomT ( void ) { }
	virtual ~BalLinMomT ( void ) { }

	/** Pure virtual functions */

	virtual void Construct ( FEA_ShapeFunctionT&, APS_MaterialT*, APS_VariableT&, APS_VariableT&, 
							int	&fTime_Step, double  fdelta_t = 0.0, int =FEA::kBackward_Euler) =0;
	virtual void Form_LHS_Keps_Kd	(	dMatrixT &Keps, dMatrixT &Kd	)	=0; 
  	virtual void Form_RHS_F_int	(	dArrayT &F_int	) =0; 
	virtual void Get ( StringT &Name, FEA_dScalarT &scalar ) =0;
	virtual void Get ( StringT &Name, FEA_dMatrixT &tensor ) =0;
  	virtual void Get ( int scalar_code, FEA_dScalarT &scalar ) =0; 

};
} // namespace Tahoe 
#endif /* _BALLINMOM_T_H_ */

