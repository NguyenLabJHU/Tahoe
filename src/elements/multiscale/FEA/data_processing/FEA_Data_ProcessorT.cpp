//DEVELOPMENT

#include "FEA.h"  

using namespace Tahoe;

//---------------------------------------------------------------------

FEA_Data_ProcessorT::FEA_Data_ProcessorT() { };

//---------------------------------------------------------------------

FEA_Data_ProcessorT::FEA_Data_ProcessorT( FEA_dMatrixT &fdNdx ) 
{
	Construct ( fdNdx );
}

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::Construct ( FEA_dMatrixT &fdNdx )  
{
  if (fdNdx.Length()==0)  
 		cout <<"...ERROR >> FEA_Data_ProcessorT::Initialize_Data_Pro : dNadx unallocated \n\n";

	  dN = fdNdx;
  n_ip = dN.IPs();
  n_sd = dN.Rows(); 
  n_en = dN.Cols();
  n_sd_x_n_en = n_sd*n_en;
  n_sd_x_n_sd = n_sd*n_sd;
  I.FEA_Dimension(n_ip,n_sd,n_sd);	
	I.Identity();
	Form_Order_Reduction_Map();
}


//---------------------------------------------------------------------

void FEA_Data_ProcessorT::Form_Order_Reduction_Map(void)
{
	Map.Dimension(n_sd,n_sd);
	int *M = Map.Pointer();
  
	if (n_sd==1)				 //  Scalar value
		(*M) = 0; 

	if (n_sd==2) {
		(*M) = 0;  M++;    //  | 1 3 | --> | 0 2 |
		(*M) = 3;  M++;    //  | 4 2 | --> | 3 1 |
		(*M) = 2;  M++;
		(*M) = 1;  
	} 
	if (n_sd==3) {
		(*M) = 0;  M++;    //  | 1 6 5 | --> | 0 5 4 |
		(*M) = 8;  M++;    //  | 9 2 4 | --> | 8 1 3 |
		(*M) = 7;  M++;    //  | 8 7 3 | --> | 7 6 2 |
		(*M) = 5;  M++;
		(*M) = 1;  M++;
		(*M) = 6;  M++;
		(*M) = 4;  M++;
		(*M) = 3;  M++;
		(*M) = 2; 
	}

};

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::grad_u	(FEA_dMatrixT &B,int T_flag) 
{
					
	int j1,j2,j3;				

	if (n_sd==1) {

  	for (int a=0; a<n_en; a++) { 	// Each interation builds a Ba

			j1 = n_sd*a;   

    	B(0,j1) = dN(dx1,a);  	// u1,1  
		}
	}

	else if (n_sd==2) {

  	for (int a=0; a<n_en; a++) { 	// Each interation builds a Ba

			j1 = n_sd*a;     j2 = n_sd*a +1;  // Columns of B for node "a"

			if (T_flag == FEA::kSymmetric) {

    		B(0,j1) = dN(dx1,a);   	B(0,j2) = 0.0;           // u1,1  
				B(1,j1) = 0.0;     			B(1,j2) = dN(dx2,a);     // u2,2 
				B(2,j1) = dN(dx2,a);    B(2,j2) = dN(dx1,a);     // u1,2 + u2,1
      	// u 1                     u 2    
			}	
			else if (T_flag == FEA::kNonSymmetric) {

    		B(0,j1) = dN(dx1,a);   	B(0,j2) = 0.0;           // u1,1  
				B(1,j1) = 0.0;     			B(1,j2) = dN(dx2,a);     // u2,2 
				B(2,j1) = dN(dx2,a);  	B(2,j2) = 0.0;           // u1,2
				B(3,j1) = 0.0;     			B(3,j2) = dN(dx1,a);     // u2,1
      	// u 1                     u 2             
			}
			else if (T_flag == FEA::kNonSymTranspose) {

    		B(0,j1) = dN(dx1,a);   	B(0,j2) = 0.0;           // u1,1  
				B(1,j1) = 0.0;     			B(1,j2) = dN(dx2,a);     // u2,2 
				B(2,j1) = 0.0;     			B(2,j2) = dN(dx1,a);     // u2,1
				B(3,j1) = dN(dx2,a);  	B(3,j2) = 0.0;           // u1,2
      	// u 1                     u 2    
			}	
			else 	cout << "...ERROR >> FEA_Data_ProcessorT::grad_u : Bad Symetry_flag \n";
  	}
	}

	else if (n_sd==3) {

  	for (int a=0; a<n_en; a++) {		// Each interation builds a Ba

			j1 = n_sd*a;     j2 = n_sd*a +1;     j3 = n_sd*a +2; 	// Columns of B for node a

			if (T_flag == FEA::kSymmetric) {

     		B(0,j1) = dN(dx1,a);  		B(0,j2) = 0.0;          	B(0,j3) = 0.0;         	 // u1,1
		 		B(1,j1) = 0.0;     				B(1,j2) = dN(dx2,a);			B(1,j3) = 0.0;         	 // u2,2
		 		B(2,j1) = 0.0;     				B(2,j2) = 0.0;          	B(2,j3) = dN(dx3,a);		 // u3,3
		 		B(3,j1) = 0.0;     				B(3,j2) = dN(dx3,a);    	B(3,j3) = dN(dx2,a);		 // u2,3 + u3,2
		 		B(4,j1) = dN(dx3,a);		  B(4,j2) = 0.0;      			B(4,j3) = dN(dx1,a);		 // u1,3 + u3,1
		 		B(5,j1) = dN(dx2,a); 		  B(5,j2) = dN(dx1,a);			B(5,j3) = 0.0;       		 // u1,2 + u2,1
     		// u 1                 		   u 2                       u 3
			}
			else if (T_flag == FEA::kNonSymmetric) {

    		B(0,j1) = dN(dx1,a);  		B(0,j2) = 0.0;            B(0,j3) = 0.0;           // u1,1  -> u0,0
				B(1,j1) = 0.0;     				B(1,j2) = dN(dx2,a);			B(1,j3) = 0.0;           // u2,2  -> u1,1
				B(2,j1) = 0.0;     				B(2,j2) = 0.0;            B(2,j3) = dN(dx3,a);		 // u3,3  -> u2,2

				B(3,j1) = 0.0;     				B(3,j2) = dN(dx3,a);    	B(3,j3) = 0.0;      		 // u2,3  -> u1,2
				B(4,j1) = dN(dx3,a);  		B(4,j2) = 0.0;      			B(4,j3) = 0.0;					 // u1,3  -> u0,2
				B(5,j1) = dN(dx2,a);  		B(5,j2) = 0.0;						B(5,j3) = 0.0;           // u1,2  -> u0,1

				B(6,j1) = 0.0;     				B(6,j2) = 0.0;				    B(6,j3) = dN(dx2,a);		 // u3,2  -> u2,1
				B(7,j1) = 0.0;   					B(7,j2) = 0.0;      			B(7,j3) = dN(dx1,a);		 // u3,1  -> u2,0
				B(8,j1) = 0.0;						B(8,j2) = dN(dx1,a);			B(8,j3) = 0.0;           // u2,1  -> u1,0
    		// u 1                  		 u 2                       u 3
			}		
			else if (T_flag == FEA::kNonSymTranspose) {

	    	B(0,j1) = dN(dx1,a);  		B(0,j2) = 0.0;            B(0,j3) = 0.0;           // u1,1  -> u0,0
				B(1,j1) = 0.0;     				B(1,j2) = dN(dx2,a);			B(1,j3) = 0.0;           // u2,2  -> u1,1
				B(2,j1) = 0.0;     				B(2,j2) = 0.0;            B(2,j3) = dN(dx3,a);		 // u3,3  -> u2,2

				B(3,j1) = 0.0;     				B(3,j2) = 0.0;				    B(3,j3) = dN(dx2,a);		 // u3,2  -> u2,1
				B(4,j1) = 0.0;   					B(4,j2) = 0.0;      			B(4,j3) = dN(dx1,a);		 // u3,1  -> u2,0
				B(5,j1) = 0.0;						B(5,j2) = dN(dx1,a);			B(5,j3) = 0.0;           // u2,1  -> u1,0

				B(6,j1) = 0.0;     				B(6,j2) = dN(dx3,a);    	B(6,j3) = 0.0;      		 // u2,3  -> u1,2
				B(7,j1) = dN(dx3,a); 			B(7,j2) = 0.0;      			B(7,j3) = 0.0;					 // u1,3  -> u0,2
				B(8,j1) = dN(dx2,a); 			B(8,j2) = 0.0;						B(8,j3) = 0.0;           // u1,2  -> u0,1
  	  	// u 1                  		 u 2                       u 3
			}
			else 	cout << "...ERROR >> FEA_Data_ProcessorT::grad_u : Bad Symetry_flag \n";
  	}
	}
	else 	cout << "...ERROR >> FEA_Data_ProcessorT::grad_u : Bad n_sd \n";

} 

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::grad_u_A (FEA_dMatrixT &A, FEA_dMatrixT &B) 
{
					
int j1,j2,j3;		

	if (n_sd==1) {

  	for (int a=0; a<n_en; a++) { 	// Each interation builds a Ba

			j1 = n_sd*a;   

    	B(0,j1) = dN(dx1,a) * A(0,0);  	// u1,1;		A is a scalar value ( i.e. a FEA_dScalarT stored in a 1x1 FEA_dMatrixT )
		}
	}
	else if (n_sd==2) {

  	for (int a=0; a<n_en; a++) {

		 	j1 = n_sd*a;     j2 = n_sd*a +1; 

	    B(k1,j1) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k1);   		B(k1,j2) = 0.0;           												// u1,1 
		 	B(k2,j1) = 0.0;     																B(k2,j2) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k2); 		// u2,2
		 	B(k3,j1) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k2);   		B(k3,j2) = 0.0;           												// u1,2
		 	B(k4,j1) = 0.0;     																B(k4,j2) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k1);  	// u2,1

   	}
	}
	else if (n_sd==3) {

   	for (int a=0; a<n_en; a++) {

		 	j1 = n_sd*a;     j2 = n_sd*a +1;     j3 = n_sd*a +2;
		 
     	B(0,j1) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k1);  B(0,j2) = 0.0;            											B(0,j3) = 0.0;          											// u1,1   
		 	B(1,j1) = 0.0;     														 B(1,j2) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k2);	B(1,j3) = 0.0;         												// u2,2   
		 	B(2,j1) = 0.0;     														 B(2,j2) = 0.0;            											B(2,j3) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k3);	// u3,3 

		 	B(3,j1) = 0.0;     														 B(3,j2) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k3);  B(3,j3) = 0.0;      													// u2,3
		 	B(4,j1) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k3);  B(4,j2) = 0.0;      														B(4,j3) = 0.0;																// u1,3
		 	B(5,j1) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k2);  B(5,j2) = 0.0;																	B(5,j3) = 0.0;          											// u1,2 

		 	B(6,j1) = 0.0;     														 B(6,j2) = 0.0;				    											B(6,j3) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k2); // u3,2
		 	B(7,j1) = 0.0;   															 B(7,j2) = 0.0;      														B(7,j3) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k1);	// u3,1
		 	B(8,j1) = 0.0;				 												 B(8,j2) = dN.Dot(FEA::kCol,a,A,FEA::kCol,k1); 	B(8,j3) = 0.0;          											// u2,1

   	}
	}
	else
		cout << "...ERROR >> FEA_Data_ProcessorT::grad_u_A : Bad n_sd \n";
}

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::A_grad_u_T (FEA_dMatrixT &A, FEA_dMatrixT &B) 
{
					
int j1,j2,j3;			

	if (n_sd==1) {

  	for (int a=0; a<n_en; a++) { 	// Each interation builds a Ba

			j1 = n_sd*a;   

    	B(0,j1) = A(0,0) * dN(dx1,a);  	// A is a scalar value (i.e. a FEA_dScalarT stored in a 1x1 FEA_dMatrixT); u1,1 scalar = u1,1^T  
		}
	}
	else if (n_sd==2) {

  	for (int a=0; a<n_en; a++) {

			j1 = n_sd*a;     j2 = n_sd*a +1; 

	    B(k1,j1) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k1);   		B(k1,j2) = 0.0;           							
			B(k2,j1) = 0.0;     																B(k2,j2) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k2); 	
			B(k3,j1) = 0.0;     																B(k3,j2) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k1);  	
			B(k4,j1) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k2);   		B(k4,j2) = 0.0;           							

   	}
	}
	else if (n_sd==3) {

   	for (int a=0; a<n_en; a++) {

			j1 = n_sd*a;     j2 = n_sd*a +1;     j3 = n_sd*a +2;
		 
    	B(0,j1) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k1);  B(0,j2) = 0.0;            											B(0,j3) = 0.0;          										  // u1,1   
			B(1,j1) = 0.0;     														 B(1,j2) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k2);	B(1,j3) = 0.0;         								 			  // u2,2   
			B(2,j1) = 0.0;     														 B(2,j2) = 0.0;            											B(2,j3) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k3);	// u3,3 

			B(3,j1) = 0.0;     														 B(3,j2) = 0.0;				    											B(3,j3) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k2); // u3,2
			B(4,j1) = 0.0;   															 B(4,j2) = 0.0;      														B(4,j3) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k1);	// u3,1
			B(5,j1) = 0.0;				 												 B(5,j2) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k1); 	B(5,j3) = 0.0;          											// u2,1

			B(6,j1) = 0.0;     														 B(6,j2) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k3);  B(6,j3) = 0.0;      													// u2,3
			B(7,j1) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k3);  B(7,j2) = 0.0;      														B(7,j3) = 0.0;																// u1,3
			B(8,j1) = dN.Dot(FEA::kCol,a,A,FEA::kRow,k2);  B(8,j2) = 0.0;																	B(8,j3) = 0.0;          											// u1,2 

   	}
	}
	else
		cout << "...ERROR >> FEA_Data_ProcessorT::grad_u : Bad n_sd \n";

}

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::A_grad_u (FEA_dMatrixT &A, FEA_dMatrixT &B, int T_flag) 
{
	int a,i,j; 
	FEA_dMatrixT Ba_1hat;
	FEA_dMatrixT AT; 
	FEA_dMatrixT Ba_2	(n_ip, n_sd_x_n_sd, n_sd);

	if (T_flag==FEA::kNonSymTranspose) {
		AT.FEA_Dimension (n_ip, n_sd, n_sd);
		AT.Transpose ( A );
	}

	grad_u ( B, T_flag ); // Is actually now either B_1hat or B_tau_1hat depending on T_flag (use B to save space)

 	for (a=0; a<n_en; a++) {

  	i = k1;  j = n_sd*a;     
		
  	Ba_1hat.FEA_Set 		(n_sd_x_n_sd,n_sd, B,i,j); 	// Construction with shallow copy of sub-matrix of B
		if (T_flag==FEA::kNonSymTranspose) 
			Ba_2.MultAB 			( Ba_1hat, AT  ); 					// Ba_2 is Ba_tau_2hat 
		else
			Ba_2.MultAB 			( Ba_1hat, A   ); 					// Ba_2 is Ba_2bar 
		B.Insert_SubMatrix 	( i,j, Ba_2 ); 							// Recall in general, A*= B can't be done w/o a temp matrix
  	Ba_1hat.FEA_UnSet 	();													// UnSet so that Components of B not freed when A3 leaves scope

	}
// returns either B_tau_2hat or B_2bar (Depending on T_flag)
}  

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::Reduce_Order	(	FEA_dMatrixT &A, 	FEA_dVectorT &a )
{
	int i,j,l;
	for (l=0; l<n_ip; l++)
		for (i=0; i<n_sd; i++)
			for (j=0; j<n_sd; j++)
				a[l][ Map(i,j) ] = A[l](i,j);

// a( Map(i,j) ) = A(i,j); <-- Future work, get this to work (i.e. components of a vector return EquateT also) 
};

//---------------------------------------------------------------------

// NOTE: A_grad_U_B <==> AikUk,lBlj = AikBljUki,l =: AAijklUkl = AA:grad_U = [ B_3hat.d ]^matrix

void FEA_Data_ProcessorT::AikBlj_Ukl (FEA_dMatrixT &A, 	FEA_dMatrixT &B, 	FEA_dMatrixT &B_3hat, int T_flag) 
{
  FEA_dMatrixT C (n_ip, n_sd_x_n_sd, n_sd_x_n_sd);
  FEA_dMatrixT B_1hat (n_ip, n_sd_x_n_sd, n_sd_x_n_en);

	grad_u ( B_1hat, T_flag );

	int i,j,k,l;

	for (i=0; i<n_sd; i++)
		for (j=0; j<n_sd; j++)
			for (k=0; k<n_sd; k++)
				for (l=0; l<n_sd; l++)
					C ( Map(i,j), Map(k,l) ) = A(i,k)*B(l,j);

	B_3hat.MultAB ( C,B_1hat );
};


//---------------------------------------------------------------------

void FEA_Data_ProcessorT::A_o_B_grad_u (FEA_dMatrixT &A, 	FEA_dMatrixT &B, 	FEA_dMatrixT &B_sharp) 
{
  FEA_dMatrixT C (n_ip, n_sd_x_n_sd, n_sd_x_n_sd);
	A_o_B( A,B,C);
  FEA_dMatrixT B_1hat (n_ip, n_sd_x_n_sd, n_sd_x_n_en);
	grad_u ( B_1hat );
	B_sharp.MultAB ( C,B_1hat );
};

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::A_o_B (FEA_dMatrixT &A, 	FEA_dMatrixT &B, 	FEA_dMatrixT &C) 
{
	int i,j,k,l; // Ensure C is a 9x9 not a 3x3

	for (i=0; i<n_sd; i++)
		for (j=0; j<n_sd; j++)
			for (k=0; k<n_sd; k++)
				for (l=0; l<n_sd; l++)
					C ( Map(i,j), Map(k,l) ) = A(i,j)*B(k,l);
};

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::II_minus_A_o_B (FEA_dMatrixT &A, FEA_dMatrixT &B, FEA_dMatrixT &P) 
{
	A_o_B (A,B,P);
  P *= -1.0;	
	FEA_dMatrixT II(n_ip, n_sd_x_n_sd, n_sd_x_n_sd); 
  IIsym(II);
  P += II;
};

//---------------------------------------------------------------------
// 4th Order Symetric Identity Tensor (Reduced to 2nd Order)
// II:A = sym(A) |---> II.[A}^vec = sym( [A]^vec 

void FEA_Data_ProcessorT::IIsym	(FEA_dMatrixT &II) 
{
	II.Identity();

	if (n_sd==1) // No such thing as sym(scalar), return a 1x1 value 1.0 
		return;

	if (n_sd==2) {
		II(2,2) = 0.5;
		II(3,3) = 0.5;
		II(2,3) = 0.5;
		II(3,2) = 0.5;
	}
	if (n_sd==3) {
    for (int i=3; i<II.Cols(); i++)
			II(i,i) = 0.5;
		II(3,6) = 0.5;
		II(4,7) = 0.5;
		II(5,8) = 0.5;
		II(6,3) = 0.5;
		II(7,4) = 0.5;
		II(8,5) = 0.5;
	}
};

//---------------------------------------------------------------------
// 1st Order Idenity Vector (This is an order reduction of the Identity Matrix,
// saves hassle of using Map.)

void FEA_Data_ProcessorT::Identity	(FEA_dVectorT &I_vec, double scale)
{
	int l,i;
	double *p = I_vec[0].Pointer();

	for (l=0; l<n_ip; l++) {
		for (i=0; i<n_sd; i++)  
			(*p++) = scale; 
		for (i=n_sd; i<I_vec.n_sd; i++)  
			(*p++) = 0.0; 
	}
}	

//---------------------------------------------------------------------
// 4th Order Elasticity Tensor CC (Reduced to 2nd Order)
// CC |---> [CC]^matrix =: D 

void FEA_Data_ProcessorT::C_IJKL (const double &lamda,const double &mu,FEA_dMatrixT &D, int kine) 
{
	double Lamda, two_mu = 2.0*mu;
	int i,j, dim = D[0].Rows(); // A Smart method, detects a 3x3,4x4,6x6 or 9x9 ?
	D = 0.0;

	if (n_sd==1) { D(0,0) = mu*(3*lamda+2*mu) / (lamda + mu);  return; } // This is Youngs Modulus E

	if (kine==FEA::kPlaneStress)  // 2D Plane Stress
		Lamda = lamda*two_mu / (lamda + two_mu);
	else  // 2D Plane Strain or 3D
    Lamda = lamda;
	
  for (i=0; i<n_sd; i++)
  	for (j=0; j<n_sd; j++)
		  D(i,j) = Lamda;
  for (i=0; i<n_sd; i++)
		D(i,i) += two_mu; 
  for (i=n_sd; i<dim; i++)
		D(i,i) = mu; 
	
};

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::C_IJKL_E_KL	(double &lamda,double &mu, FEA_dMatrixT &E, FEA_dMatrixT &S) // Hooke's Law // Untested
{
FEA_dScalarT lamda_x_trE(n_ip); 
E.Trace( lamda_x_trE );
lamda_x_trE *= lamda;

if (n_sd==1) { S(0,0) = E(0,0) * ( mu*(3*lamda+2*mu) / (lamda + mu) ); return; }  // 1D Hooke:  stress = E*strain

S = E;
S *= 2.0*mu;
for (int i=0; i<n_sd; i++)
	S(i,i) += lamda_x_trE; 
};

//---------------------------------------------------------------------

void FEA_Data_ProcessorT::c_ijkl	(double &lamda,double &mu, FEA_dScalarT &J, FEA_dMatrixT &F, FEA_dMatrixT &D) 
{
	int i,j,k,l;
	FEA_dMatrixT b	(n_ip,n_sd,n_sd); // Left Cauchy Tensor
	b.MultABT(F,F);
 
	for (i=0; i<n_sd; i++)
		for (j=0; j<n_sd; j++)
			for (k=0; k<n_sd; k++)
				for (l=0; l<n_sd; l++) 
					D ( Map(i,j), Map(k,l) )  = ( b(i,j)*b(k,l)*lamda + ( b(i,k)*b(j,l) + b(i,l)*b(j,k) )*mu ) /J; 
}

//---------------------------------------------------------------------------

/** Given a matrix T evaluated at element nodal points, return Curl(T) 
 *  evaluated at each of the integration points.  Use another utility
 *  to map back to nodes if desired. */

void	FEA_Data_ProcessorT::Curl	(const ArrayT<dMatrixT>& T, FEA_dMatrixT& curl) const
{ 

  int a,l,n_en=T.Length(); 
  double* zero_ptr; 
	dMatrixT zero;

 	if (n_sd==2) { // This is the spatial dimension of T. Recall n_sd(curl(T)) = 3 always
  	zero.Dimension(n_sd,n_en);
		zero = 0.0;
		zero_ptr = zero.Pointer(); 
	}

	cout << "n_ip" << n_ip << "\n\n";

	for (l=0; l<n_ip; l++) {

		double* pcurl = curl[l].Pointer();

		double& c11 = *pcurl++;
		double& c21 = *pcurl++;
		double& c31 = *pcurl++;
		double& c12 = *pcurl++;
		double& c22 = *pcurl++;
		double& c32 = *pcurl++;
		double& c13 = *pcurl++;
		double& c23 = *pcurl++;
		double& c33 = *pcurl  ;
	
		c11 = c21 = c31 = c12 = c22 = c32 = c13 = c23 = c33 = 0.0;

		double *dx1 = dN[l].Pointer(0); 
		double *dx2 = dx1+1; 
		double *dx3 = dx1+2;

		if (n_sd==2)  
			dx3 = zero_ptr; 

		for (a=0; a<n_en; a++) {
	
		 	double *pT = T[a].Pointer();

		  double& T11 = *pT++;
			double& T21 = *pT++;
		  double& T31 = *pT++;
		  double& T12 = *pT++;
		 	double& T22 = *pT++;
		 	double& T32 = *pT++;
		 	double& T13 = *pT++;
		 	double& T23 = *pT++;
		 	double& T33 = *pT  ;

		 	c11 += ( T12*(*dx3) - T13*(*dx2) );
		 	c21 += ( T22*(*dx3) - T23*(*dx2) );
		 	c31 += ( T32*(*dx3) - T33*(*dx2) );

	  	c12 += ( T13*(*dx1) - T11*(*dx3) );
	  	c22 += ( T23*(*dx1) - T21*(*dx3) );
	  	c32 += ( T33*(*dx1) - T31*(*dx3) );

	  	c13 += ( T11*(*dx2) - T12*(*dx1) );
	  	c23 += ( T21*(*dx2) - T22*(*dx1) );
	  	c33 += ( T31*(*dx2) - T32*(*dx1) );
			
	  	dx1+=n_sd; dx2+=n_sd; dx3+=n_sd;	// Goto next column: n_sd = num rows
		}

	}

}


//----------------------------------------------- End

















