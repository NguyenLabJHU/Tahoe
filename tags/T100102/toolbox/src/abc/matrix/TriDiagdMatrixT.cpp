/* $Id: TriDiagdMatrixT.cpp,v 1.4 2002-09-12 16:40:17 paklein Exp $ */
/* created: paklein (01/15/1998)                                          */
/* Triadiagonal matrix with Gauss elimination. The                        */
/* matrix is stored in row major form.                                    */

#include "TriDiagdMatrixT.h"
#include <math.h>
#include "toolboxConstants.h"
#include "dArrayT.h"

/* constructor */

using namespace Tahoe;

TriDiagdMatrixT::TriDiagdMatrixT(int rows):
	nArrayT<double>(rows*3),
	fRows(rows),
	pL(Pointer()),
	pD(pL + fRows),
	pR(pD + fRows)
{

}

/* Gaussian elimination with the given RHS vector or RHS matrix */
void TriDiagdMatrixT::LinearSolve(dArrayT& RHS)
{
	/* dimension check */
	if (RHS.Length() != fRows) throw eSizeMismatch;
	
	/* quick exit */
	if (fRows == 0) return;

	double *pdiag  = pD;
	double *upper  = pR;
	double *pdiag1 = pD + 1;
	double *pzero  = pL + 1;
	
	double *pRHS  = RHS.Pointer();
	double *pRHS1 = pRHS + 1;
	
	/* forward reduction */
	for (int i = 1; i < fRows; i++)
	{
#if __option(extended_errorcheck)
		if (fabs(*pdiag) < kSmall) throw eGeneralFail;
#endif

		double factor = (*pzero++)/(*pdiag++);

		(*pdiag1++) -= (*upper++)*factor;	
		(*pRHS1++)   -= (*pRHS++)*factor;
	}
	
	/* reset pointers */
	pdiag--;
	upper--;
	pdiag1--;
	pRHS--;
	pRHS1--;
	
	/* back substitution */
#if __option(extended_errorcheck)
	if (fabs(*pdiag1) < kSmall) throw eGeneralFail;
#endif
	*pRHS1 /= *pdiag1;
	for (int j = 1; j < fRows; j++)
	{
#if __option(extended_errorcheck)
	if (fabs(*pdiag) < kSmall) throw eGeneralFail;
#endif

		(*pRHS)   -= (*upper--)*(*pRHS1--);
		(*pRHS--) /= (*pdiag--);
	}
}
