// $Id: APS_VariableT.cpp,v 1.6 2003-09-19 00:47:02 raregue Exp $
#include "APS_VariableT.h"

//---------------------------------------------------------------------
/** constructor */

using namespace Tahoe;

APS_VariableT::APS_VariableT (const FEA_dMatrixT& grad_u, const FEA_dMatrixT& grad_gammap) 
{
	Construct	(grad_u, grad_gammap);
}

//---------------------------------------------------------------------
/** Data initialization: Allocate space for grad_u, grad_gammap  */

void APS_VariableT::Construct (const FEA_dMatrixT& grad_u, const FEA_dMatrixT& grad_gammap) 
{
  	n_vars = APS::kNUM_APS_VARS;  
  	fVars.Dimension( n_vars );  

	fVars[APS::kgrad_u] = grad_u; // This = opr allocates if LHS Length=0
	fVars[APS::kgrad_gammap] = grad_gammap; 
}

//----------------------------------------------------

void APS_VariableT::Delete_Vars	( void )
{
#if 0
	for (int i=0; i<n_vars; i++) 
		fVars[i].FEA_Delete(); // ArrayT checks if fLength=0 before deleting
#endif
#pragma message("APS_VariableT::Delete_Vars: fVars[i] is a FEA_dVectorT which has no FEA_Delete()")
}

//----------------------------------------------------

void APS_VariableT::Print() { Print(" "); } 

void APS_VariableT::Print(char *c) { // overload << later

  cout <<"\n APS_VariableT "<< c <<" follows: \n\n"; 

	for (int l=0; l<n_vars; l++) 
		if (fVars[l].IPs() == 0)
			cout << "APS_VariableT n["<<l<<"] Unallocated \n\n";
    else {
  		cout << "\n Vector "<<l<<" evaluated at "<<fVars[l].IPs() <<" integration points (ip): \n"; 
  		for (int i=0; i<fVars[l].IPs(); i++) 
   	 		cout <<"\n "<< c <<" @ ip "<<i<<": \n\n"<< fVars[l][i] << "\n";
			cout << "\n";
		}
	
}

//---------------------------------------------------------------------
//** Retrieve/Fetch/Get either grad_u or gammap from class work space
/*const FEA_dMatrixT& APS_VariableT::Get(APS::VarT variable)  
{

  if (!fVars[variable].Length())  // 0 rows indicated un-allocation
    Allocate_and_Compute_Variables(variable);

  return fVars[variable];
  
} */

//---------------------------------------------------------------------

void APS_VariableT::operator=(const APS_VariableT &a)	// Initializes
{
	n_vars = a.n_vars;
	fVars.Dimension( n_vars );

//#if 0
	for (int i=0; i<n_vars; i++) {
  	fVars[i].FEA_Dimension( a.fVars[i].IPs(), a.fVars[i].Rows(), a.fVars[i].Cols() );
  	fVars[i] = a.fVars[i];
	}
//#endif
//#pragma message("APS_VariableT::operator=: fVars[i] is a FEA_dVectorT which has no n_rows")
};

//---------------------------------------------------------------------

void APS_VariableT::operator +=  (const double &a)
{
for (int i=0; i<n_vars; i++) fVars[i] += a;
};


//---------------------------------------------------------------------

void APS_VariableT::operator -=  (const double &a)
{
for (int i=0; i<n_vars; i++) fVars[i] -= a;
};

//---------------------------------------------------------------------

void APS_VariableT::operator *=  (const double &a)
{
for (int i=0; i<n_vars; i++) fVars[i] *= a;
};

//---------------------------------------------------------------------

void APS_VariableT::operator /=  (const double &a) 
{
for (int i=0; i<n_vars; i++) fVars[i] /= a;
};

//---------------------------------------------------------------------

void APS_VariableT::SumOf (APS_VariableT &a,APS_VariableT &b) // Initializes
{
n_vars = a.n_vars;
fVars.Dimension( n_vars );

//#if 0
for (int i=0; i<n_vars; i++) {
	fVars[i].FEA_Dimension( a.fVars[i].IPs(), a.fVars[i].Rows(), a.fVars[i].Cols() );
	fVars[i].SumOf(a.fVars[i],b.fVars[i]); 
}
//#endif
//#pragma message("APS_VariableT::SumOf: fVars[i] is a FEA_dVectorT which has no n_rows")
};

//---------------------------------------------------------------------
