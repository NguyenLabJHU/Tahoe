// $Id: MFGP_MFA_Data_Processor_DisplT.h
#ifndef _MFGP_MFA_DATAPROCESSOR_DISPLT_H_
#define _MFGP_MFA_DATAPROCESSOR_DISPLT_H_

#include "MeshFreeT.h"
#include "MLSSolverGPT.h"

// Pass nodal info, first and third derivatives
// of the shape function of displacement to this class. 
// Compute B1 and B3
// Dimension?? 
namespace Tahoe 
{

class MFGP_MFA_Data_Processor_DisplT  
	{
	public:

		 MFGP_MFA_Data_Processor_DisplT(); 
		~MFGP_MFA_Data_Processor_DisplT(); 
		 
		MFGP_MFA_Data_Processor_DisplT ( dArray2DT &fdNdx, dArray2DT &fd3Ndx3 ); //   ??MLSSolverGPT::SetShapeFunctions(const dArrayT& volume) )
		
		void Construct ( dArray2DT &fdNdx, dArray2DT &fd3Ndx3 ); //   ??MLSSolverGPT::SetShapeFunctions(const dArrayT& volume) )
        
		void Set_B1 (dMatrixT &B1);
		void Set_B3 (dMatrixT &B3);
		
		dArray2DT dN, d3N;
	};

}

// inline routines go here ...

#endif /* _MFGP_MFA_DATAPROCESSOR_DISPLT_H_ */
