// $Id: MFGP_Con_EqT.h
#ifndef _MFGP_CON_EQ_T_H_ 
#define _MFGP_CON_EQ_T_H_ 

#include "MFGP_PlastT.h"

namespace Tahoe 
{

/** MFGP_Con_EqT: This class contains methods which build stiffness matricies 
 *  and formulate the non-linear Newton-Raphson equations Kd = -R
 *  for a coupled approach to implementation of 
 *  the Consistency Condition in weak form 
 **/


class MFGP_Con_EqT : public MFGP_PlastT
{

public:
	
 	MFGP_Con_EqT	( void ) { }
								
	MFGP_Con_EqT	( int&, MeshFreeShapeFunctionT&, MeshFreeShapeFunctionT&, GRAD_MRSSKStV&,
				int &fTime_Step, double fdelta_t = 0.0);

	void  	Initialize	( int &in_sd, int &in_en_displ, int &in_en_plast, int &in_state, int &in_str, 
							int Initial_Time_Step=1 );

	void 	Construct 	( int&, MeshFreeShapeFunctionT&, MeshFreeShapeFunctionT&, GRAD_MRSSKStV&,  
						int &fTime_Step, double fdelta_t = 0.0); 
  	void 	Form_LHS_Ku_Klambda	( dMatrixT &Ku, dMatrixT &Klambda ); 
  	void 	Form_RHS_F_int		( dArrayT &F_int ); 
	void 	Form_B_List 		( void );  // Strain Displacement Matricies
 	void 	Form_C_List 		( GRAD_MRSSKStV &GRAD_MR_Plast_Mat);  // Constant List

	
	protected:

		// check the dimensions!!
	  	dMatrixT B1_d, B3_d; 
		dMatrixT B4_lam, phi_lam; //dimension??
		//double B4_lam, phi_lam; //dimension??
	  	dMatrixT Cuu1, Cuu2, Culam1, Culam2;
  		dMatrixT Clamu1, Clamu2;
  		double Clamlam1, Clamlam2;
  		double yield;
			
	protected:

		MFGP_MFA_Data_Processor_DisplT Data_Pro_Displ; 
		MFGP_MFA_Data_Processor_PlastT Data_Pro_Plast; 

		int ip, n_rows, n_cols, n_sd, n_en_displ, n_en_plast, n_sd_x_n_sd, n_sd_x_n_en_plast, Time_Integration_Scheme;
		int time_step, n_state, n_str;

		double delta_t;
};

} // namespace Tahoe 
#endif /* _MFGP_CON_EQ_T_H_ */

