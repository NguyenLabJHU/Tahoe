
#ifndef _FEA_FORMATT_
#define _FEA_FORMATT_

#include "FEA.h"
#include "ShapeFunctionT.h"
#include "dArrayT.h"
#include "LocalArrayT.h"

namespace Tahoe {

class	FEA_FormatT {

	public:

		FEA_FormatT (void) { }

		void Shapes					(	ShapeFunctionT *fShapes, FEA_ShapeFunctionT &FEA_Shapes 							);
		void Na							(	int n_en, ShapeFunctionT *fShapes, FEA_ShapeFunctionT &FEA_Shapes 		);
		void Gradients 			( ShapeFunctionT*,LocalArrayT&,LocalArrayT&,FEA_dMatrixT&,FEA_dMatrixT&	);
		void Displacements 	(	LocalArrayT &u_mat, dArrayT &u_vec 																		);

};

}

#endif



