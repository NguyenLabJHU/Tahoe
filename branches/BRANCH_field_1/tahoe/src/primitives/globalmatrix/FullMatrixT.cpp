/* $Id: FullMatrixT.cpp,v 1.7 2002-04-10 01:12:03 paklein Exp $ */
/* created: paklein (03/07/1998) */

#include "FullMatrixT.h"
#include <iostream.h>
#include <iomanip.h>
#include "Constants.h"
#include "dArrayT.h"
#include "iArrayT.h"
#include "ElementMatrixT.h"

/* constructor */
FullMatrixT::FullMatrixT(ostream& out,int check_code):
	GlobalMatrixT(out, check_code)
{

}

/* copy constructor */
FullMatrixT::FullMatrixT(const FullMatrixT& source):
	GlobalMatrixT(source),
	fMatrix(source.fMatrix)
{

}


/* set the internal matrix structure.
* NOTE: do not call Initialize() equation topology has been set
* with AddEquationSet() for all equation sets */
void FullMatrixT::Initialize(int tot_num_eq, int loc_num_eq, int start_eq)
{
	/* inherited */
	GlobalMatrixT::Initialize(tot_num_eq, loc_num_eq, start_eq);

	/* check */
	if (tot_num_eq != loc_num_eq)
	{
		cout << "\n FullMatrixT::Initialize: expecting total number of equations\n"
		     <<   "     " << tot_num_eq
		     << " to be equal to the local number of equations " << loc_num_eq << endl;
		throw eGeneralFail;
	}
	
	/* allocate work space */
	fMatrix.Allocate(fLocNumEQ);
}

/* set all matrix values to 0.0 */
void FullMatrixT::Clear(void)
{
	/* inherited */
	GlobalMatrixT::Clear();
	
	/* clear values */
	fMatrix = 0.0;	
}

/* add element group equations to the overall topology.
* NOTE: assembly positions (equation numbers) = 1...fNumEQ
* equations can be of fixed size (iArray2DT) or
* variable length (RaggedArray2DT) */
void FullMatrixT::AddEquationSet(const iArray2DT& eqset)
{
#pragma unused(eqset)
// no equation data needed
}

void FullMatrixT::AddEquationSet(const RaggedArray2DT<int>& eqset)
{
#pragma unused(eqset)
// no equation data needed
}

/* assemble the element contribution into the LHS matrix - assumes
* that elMat is square (n x n) and that eqnos is also length n.
* NOTE: assembly positions (equation numbers) = 1...fNumEQ */
void FullMatrixT::Assemble(const ElementMatrixT& elMat, const nArrayT<int>& eqnos)
{
#if __option(extended_errorcheck)
	/* dimension checking */
	if (elMat.Rows() != eqnos.Length() ||
	    elMat.Cols() != eqnos.Length()) throw eSizeMismatch;
#endif

	/* element matrix format */
	ElementMatrixT::FormatT format = elMat.Format();

	if (format == ElementMatrixT::kDiagonal)
	{
		/* from diagonal only */
		double* pelMat = elMat.Pointer();
		int inc = elMat.Rows() + 1;
	
		int*    peq    = eqnos.Pointer();		
		for (int i = 0; i < elMat.Length(); i++)
		{
			int eq = *peq++;
			
			/* active dof's only */
			if (eq-- > 0) fMatrix(eq,eq) += *pelMat;
			
			pelMat += inc;
		}
	}
	else
	{
		/* copy to full symmetric */
		if (elMat.Format() == ElementMatrixT::kSymmetricUpper) elMat.CopySymmetric();

		int*    peq    = eqnos.Pointer();	
		for (int col = 0; col < elMat.Cols(); col++)
		{
			int        eqc = eqnos[col];
			double* pelMat = elMat(col);
		
			/* active dof's only */
			if (eqc-- > 0)
			{		
				int* peqr = eqnos.Pointer();
						
				for (int row = 0; row < elMat.Rows(); row++)
				{
					int eqr = *peqr++;
				
					/* active dof's only */
					if (eqr-- > 0) fMatrix(eqr,eqc) += *pelMat;
					
					pelMat++;
				}
			}
		}
	}
}

void FullMatrixT::Assemble(const ElementMatrixT& elMat, const nArrayT<int>& row_eqnos,
	const nArrayT<int>& col_eqnos)
{
#if __option(extended_errorcheck)
	/* dimension check */
	if (elMat.Rows() != row_eqnos.Length() ||
	    elMat.Cols() != col_eqnos.Length()) throw eSizeMismatch;
#endif

	/* element matrix format */
	ElementMatrixT::FormatT format = elMat.Format();

	if (format == ElementMatrixT::kDiagonal)
	{
		cout << "\n FullMatrixT::Assemble(m, r, c): cannot assemble diagonal matrix" << endl;
		throw eGeneralFail;
	}
	else
	{
		/* copy to full symmetric */
		if (elMat.Format() == ElementMatrixT::kSymmetricUpper) elMat.CopySymmetric();

		/* assemble active degrees of freedom */
		int n_c = col_eqnos.Length();
		int n_r = row_eqnos.Length();
		for (int col = 0; col < n_c; col++)
		{
			int ceqno = col_eqnos[col] - 1;	
			if (ceqno > -1)	
				for (int row = 0; row < n_r; row++)
				{
					int reqno = row_eqnos[row] - 1;
					if (reqno > -1)
						fMatrix(reqno,ceqno) += elMat(row,col);
				}
		}
	}
}

/* strong manipulation functions */
void FullMatrixT::OverWrite(const ElementMatrixT& elMat, const nArrayT<int>& eqnos)
{
#if __option(extended_errorcheck)
	/* dimension checking */
	if (elMat.Rows() != eqnos.Length() ||
	    elMat.Cols() != eqnos.Length()) throw eSizeMismatch;
#endif

	/* copy to full symmetric */
	if (elMat.Format() == ElementMatrixT::kSymmetricUpper) elMat.CopySymmetric();

	int* peq = eqnos.Pointer();	
	for (int col = 0; col < elMat.Cols(); col++)
	{
		int        eqc = eqnos[col];
		double* pelMat = elMat(col);
		
		/* active dof's only */
		if (eqc-- > 0)
		{		
			int* peqr = eqnos.Pointer();
					
			for (int row = 0; row < elMat.Rows(); row++)
			{
				int eqr = *peqr++;
			
				/* active dof's only */
				if (eqr-- > 0) fMatrix(eqr,eqc) += *pelMat;
				
				pelMat++;
			}
		}
	}
}

void FullMatrixT::Disassemble(dMatrixT& elMat, const nArrayT<int>& eqnos) const
{
#if __option(extended_errorcheck)
	/* dimension checking */
	if (elMat.Rows() != eqnos.Length() ||
	    elMat.Cols() != eqnos.Length()) throw eSizeMismatch;
#endif

	int* peq = eqnos.Pointer();
	
	for (int col = 0; col < elMat.Cols(); col++)
	{
		int        eqc = eqnos[col];
		double* pelMat = elMat(0);
		
		/* active dof's only */
		if (eqc-- > 0)
		{		
			int* peqr = eqnos.Pointer();
					
			for (int row = 0; row < elMat.Rows(); row++)
			{
				int eqr = *peqr++;
			
				/* active dof's only */
				if (eqr-- > 0)
					*pelMat = fMatrix(eqr,eqc);
				else
					*pelMat = 0.0;
				
				pelMat++;
			}
		}
		else
			/* clear column */
			elMat.SetCol(col, 0.0);
	}
}

void FullMatrixT::DisassembleDiagonal(dArrayT& diagonals, const nArrayT<int>& eqnos) const
{
#if __option(extended_errorcheck)
	/* dimension checking */
	if (diagonals.Length() != eqnos.Length()) throw eSizeMismatch;
#endif

	for (int i = 0; i < eqnos.Length(); i++)
	{
		/* ignore requests for inactive equations */	
		int eq = eqnos[i];	
		if (eq-- > 0)
			diagonals[i] = fMatrix(eq,eq);
		else
			diagonals[i] = 0.0;
	}
}

/* number scope and reordering */
GlobalMatrixT::EquationNumberScopeT FullMatrixT::EquationNumberScope(void) const
{
	return kLocal;
}

bool FullMatrixT::RenumberEquations(void) const { return false; }

/* assignment operator */
GlobalMatrixT& FullMatrixT::operator=(const FullMatrixT& rhs)
{
	/* inherited */
	GlobalMatrixT::operator=(rhs);

	fMatrix = rhs.fMatrix;
	return *this;
}

/* assignment operator */
GlobalMatrixT& FullMatrixT::operator=(const GlobalMatrixT& rhs)
{
#ifdef __NO_RTTI__
	cout << "\n FullMatrixT::operator= : requires RTTI" << endl;
	throw eGeneralFail;
#endif

	const FullMatrixT* full = dynamic_cast<const FullMatrixT*>(&rhs);
	if (!full) {
		cout << "\n FullMatrixT::operator= : cast failed" << endl;
		throw eGeneralFail;
	}
	return operator=(*full);
}
	
/* return a clone of self. Caller is responsible for disposing of the matrix */
GlobalMatrixT* FullMatrixT::Clone(void) const
{
	FullMatrixT* new_mat = new FullMatrixT(*this);
	return new_mat;
}

bool FullMatrixT::Multx(const dArrayT& x, dArrayT& b) const
{
	/* already factorized */
	if (fIsFactorized) {
		cout << "\n FullMatrixT::Multx: cannot calculate product with factorized matrix" << endl;
		return false;
	} else {
		/* calculate product */
		fMatrix.Multx(x, b);
		return true;
	}
}

bool FullMatrixT::MultTx(const dArrayT& x, dArrayT& b) const
{
	/* already factorized */
	if (fIsFactorized) {
		cout << "\n FullMatrixT::MultTx: cannot calculate product with factorized matrix" << endl;
		return false;
	} else {
		/* calculate product */
		fMatrix.MultTx(x, b);
		return true;
	}
}

/**************************************************************************
* Protected
**************************************************************************/

/* precondition matrix */
void FullMatrixT::Factorize(void)
{
//TEMP: no full, nonsymmetric factorization implemented
	fIsFactorized = 0; // undo GlobalMatrixT flag set
}
	
/* determine new search direction and put the results in result */
void FullMatrixT::BackSubstitute(dArrayT& result)
{
//TEMP: no full, nonsymmetric factorization implemented
	if (fIsFactorized) throw eGeneralFail;

	fMatrix.LinearSolve(result);
	fIsFactorized = true;
}

/* rank check functions */
void FullMatrixT::PrintAllPivots(void) const
{
//TEMP: no full, nonsymmetric factorization implemented
}

void FullMatrixT::PrintZeroPivots(void) const
{
//TEMP: no full, nonsymmetric factorization implemented
}

void FullMatrixT::PrintLHS(void) const
{
	if (fCheckCode != GlobalMatrixT::kPrintLHS) return;
		
	fOut << "\nLHS matrix:\n\n";
	fOut << fMatrix << "\n\n";
}
