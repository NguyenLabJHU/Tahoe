//DEVELOPMENT

#ifndef _VMS_EZ_T_H_ 
#define _VMS_EZ_T_H_ 

#include "FineScaleT.h"

namespace Tahoe {

/** VMS_EZT: This class contains methods which build stiffness matricies 
 *  and formulate the non-linear Newton-Raphson equations Kd = -R
 *  for a Variational Multi-Scale (VMS) approach to implementation of Sandia's
 *  BCJ Model.  A dual field formulation u^alpha and u^beta is used. See
 *  Creighton et. al for more. 
 *  Collaboration:  Sandia National Laboratory and the University of Michigan **/


class VMS_EZT : public FineScaleT
{

	public:

  	enum B_T { 
						   	kB_1hat,   
	             	kNUM_B_TERMS };  // <-- Use for loops and count (KEEP THIS ONE LAST!!)

    enum A_T { 
						   	kgrad_ua,  
						   	kgrad_ub,  
						   	kG2,
	             	kNUM_A_TERMS };  // <-- Use for loops and count (KEEP THIS ONE LAST!!)

		//--------------------------------------------------------------
	
 	VMS_EZT	(	) { }
								
	VMS_EZT	( FEA_ShapeFunctionT&, VMF_MaterialT*, VMS_VariableT&, VMS_VariableT&, 
														 double fdelta_t = 0.0, int IntegrationScheme = FEA::kBackward_Euler);

	void 	Construct ( FEA_ShapeFunctionT&, VMF_MaterialT*, VMS_VariableT&, VMS_VariableT&, 
								 double fdelta_t = 0.0, int Integration_Scheme = FEA::kBackward_Euler); 

  void 	Form_LHS_Ka_Kb	(	dMatrixT &Ka, dMatrixT &Kb ); 
  void 	Form_RHS_F_int	(	dArrayT &F_int ); 
	void 	Form_B_List 		( void );  // Strain Displacement Matricies
	void 	Form_A_S_Lists 	( VMS_VariableT &np1, VMS_VariableT &n ); // BCDE ---> A 

	protected:

  	FEA_dMatrix_ArrayT    B; // Dimension: n_sd x n_sd*n_en , 		e.g. 9x24
		FEA_dMatrix_ArrayT    A; // Dimension: n_sd x n_sd , 					e.g. 3x3 
 
	protected:

		FEA_IntegrationT 		Integral;
		FEA_Data_ProcessorT Data_Pro; 

		int n_ip, n_rows, n_cols, n_sd, n_en, n_sd_x_n_sd, n_sd_x_n_en;
		double neg_1by10;
  
};

} // namespace Tahoe 
#endif /* _VMS_EZT_H_ */

