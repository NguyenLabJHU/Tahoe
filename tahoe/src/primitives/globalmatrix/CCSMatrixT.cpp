/* $Id: CCSMatrixT.cpp,v 1.18 2003-12-28 08:24:00 paklein Exp $ */
/* created: paklein (05/29/1996) */
#include "CCSMatrixT.h"

#include <math.h>
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include <string.h>

#include "toolboxConstants.h"

#include "dMatrixT.h"
#include "dArrayT.h"
#include "iArrayT.h"
#include "iArray2DT.h"
#include "RaggedArray2DT.h"
#include "ElementMatrixT.h"

using namespace Tahoe;

/* constructor */
CCSMatrixT::CCSMatrixT(ostream& out, int check_code):
	GlobalMatrixT(out, check_code),
	fDiags(NULL),
	fNumberOfTerms(0),
	fMatrix(NULL)
{

}

CCSMatrixT::CCSMatrixT(const CCSMatrixT& source):
	GlobalMatrixT(source),
	fDiags(NULL),
	fNumberOfTerms(0),
	fMatrix(NULL)
{
	CCSMatrixT::operator=(source);
}

CCSMatrixT::~CCSMatrixT(void)
{
	delete[] fDiags;
	delete[] fMatrix;
}

/* set the internal matrix structure.
* NOTE: do not call Initialize() equation topology has been set
* with AddEquationSet() for all equation sets */
void CCSMatrixT::Initialize(int tot_num_eq, int loc_num_eq, int start_eq)
{
	/* inherited */
	GlobalMatrixT::Initialize(tot_num_eq, loc_num_eq, start_eq);

	/* check */
	if (tot_num_eq != loc_num_eq)
	{
		cout << "\n CCSMatrixT::Initialize: expecting total number of equations\n"
		     <<   "     " << tot_num_eq
		     << " to be equal to the local number of equations " << loc_num_eq << endl;
		throw ExceptionT::kGeneralFail;
	}

	/* allocate diagonal index array */
	if (fDiags != NULL) delete[] fDiags;
	iArrayT i_memory;
	try {
		i_memory.Dimension(fLocNumEQ);
		i_memory.ReleasePointer(&fDiags);
	}	
	catch (ExceptionT::CodeT error)
	{
		if (error == ExceptionT::kOutOfMemory)
		{
			cout << "\n CCSMatrixT::Initialize: not enough memory" << endl;
			fOut << "\n CCSMatrixT::Initialize: not enough memory" << endl;
		}
		throw error;
	}	

	/* compute matrix structure and return dimensions */
	int meanband;
	int band;
	ComputeSize(fNumberOfTerms, meanband, band);

	/* output */
	fOut << " Number of terms in global matrix. . . . . . . . = " << fNumberOfTerms << '\n';
	fOut << " Mean half bandwidth . . . . . . . . . . . . . . = " << meanband << '\n';
	fOut << " Bandwidth . . . . . . . . . . . . . . . . . . . = " << band     << '\n';

	/* allocate matrix */
	if (fMatrix != NULL) delete[] fMatrix;
	dArrayT d_memory;
	try {
		d_memory.Dimension(fNumberOfTerms);
		d_memory.ReleasePointer(&fMatrix);
	}	
	catch (ExceptionT::CodeT error)
	{
		if (error == ExceptionT::kOutOfMemory)
		{
			cout << "\n CCSMatrixT::Initialize: not enough memory" << endl;
			fOut << "\n CCSMatrixT::Initialize: not enough memory" << endl;
		}
		throw error;
	}	

	int computefilledin = 1;
	if (computefilledin)
	{
		int printRCV = 0;
		int filledelements = NumberOfFilled(printRCV);
		double percent_fill = (fNumberOfTerms != 0) ? 
			(100.0*filledelements)/fNumberOfTerms : 
			0.0; 

		fOut << " Number of non-zero values (pre-factorization) . = ";
		fOut << filledelements << '\n';
		fOut << " Storage efficiency (% non-zero) . . . . . . . . = ";
		fOut << percent_fill << '\n';
	}
	/* flush stream */
	fOut << endl;
	
	/* clear stored equation sets */
	fEqnos.Clear();
	fRaggedEqnos.Clear();
}

/* set all matrix volues to 0.0 */
void CCSMatrixT::Clear(void)
{
	/* inherited */
	GlobalMatrixT::Clear();

	/* byte set */
	memset(fMatrix, 0, sizeof(double)*fNumberOfTerms);	
}

/* add element group equations to the overall topology.
* NOTE: assembly positions (equation numbers) = 1...fNumEQ
* equations can be of fixed size (iArray2DT) or
* variable length (RaggedArray2DT) */
void CCSMatrixT::AddEquationSet(const iArray2DT& eqset)
{
	fEqnos.AppendUnique(&eqset);	
}

void CCSMatrixT::AddEquationSet(const RaggedArray2DT<int>& eqset)
{
	fRaggedEqnos.AppendUnique(&eqset);	
}

/* assemble the element contribution into the LHS matrix - assumes
* that elMat is square (n x n) and that eqnos is also length n.
* NOTE: assembly positions (equation numbers) = 1...fNumEQ */
void CCSMatrixT::Assemble(const ElementMatrixT& elMat, const ArrayT<int>& eqnos)
{
	/* element matrix format */
	ElementMatrixT::FormatT format = elMat.Format();

	if (format == ElementMatrixT::kNonSymmetric)
		throw ExceptionT::kGeneralFail;
	else if (format == ElementMatrixT::kDiagonal)
	{
		/* from diagonal only */
		const double* pelMat = elMat.Pointer();
		int inc = elMat.Rows() + 1;
		
		int size = eqnos.Length();
		int eqno;
		for (int eqdex = 0; eqdex < size; eqdex++)
		if ( (eqno = eqnos[eqdex]) > 0)	/* active dof */
		{
			int dex = fDiags[eqno-1];

#if __option (extended_errorcheck)
			if (dex < 0 || dex >= fNumberOfTerms)
			{
				cout << "\nCCSMatrixT::Assemble: index out of range: " << dex << endl;
				throw ExceptionT::kGeneralFail;
			}
#endif
			/* assemble */
			fMatrix[dex] += *pelMat;
		}
		
		pelMat += inc;
	}
	else
	{
		int size = eqnos.Length();
		int	ceqno, reqno;

		for (int col = 0; col < size; col++)
			if ( (ceqno = eqnos[col]) > 0)	/* active dof */
			{
				for (int row = 0; row <= col; row++)	/* upper triangle */
					if ( (reqno = eqnos[row]) > 0)
					{
						int dex;

						if (ceqno > reqno)
							dex = fDiags[ceqno-1] - (ceqno-reqno);
						else
							dex = fDiags[reqno-1] - (reqno-ceqno);

#if __option (extended_errorcheck)
						if (dex < 0 || dex >= fNumberOfTerms)
						{
							cout << "\nCCSMatrixT::Assemble: index out of range: " << dex << endl;
							throw ExceptionT::kGeneralFail;
						}
#endif
						/* off-diagonal in element, but global in diagonal */
						if (ceqno == reqno && row != col)
							fMatrix[dex] += 2.0*elMat(row,col);
						else
							fMatrix[dex] += elMat(row,col);
					}
			}
	}
}

void CCSMatrixT::Assemble(const ElementMatrixT& elMat, const ArrayT<int>& row_eqnos,
	const ArrayT<int>& col_eqnos)
{
#if __option(extended_errorcheck)
	/* dimension check */
	if (elMat.Rows() != row_eqnos.Length() ||
	    elMat.Cols() != col_eqnos.Length()) throw ExceptionT::kSizeMismatch;
#else
#pragma unused(row_eqnos)
#pragma unused(col_eqnos)
#endif

	/* element matrix format */
	ElementMatrixT::FormatT format = elMat.Format();

	if (format == ElementMatrixT::kNonSymmetric) 
	{
		cout << "\n CCSMatrixT::Assemble(m, r, c): element matrix is not symmetric" << endl;
		throw ExceptionT::kGeneralFail;	
	}
	else if (format == ElementMatrixT::kDiagonal)
	{
		cout << "\n CCSMatrixT::Assemble(m, r, c): cannot assemble diagonal matrix" << endl;
		throw ExceptionT::kGeneralFail;
	}
	else
	{
		cout << "\n CCSMatrixT::Assemble(m, r, c): cannot assemble symmetric matrix" << endl;
		throw ExceptionT::kGeneralFail;
	}
}

void CCSMatrixT::Assemble(const nArrayT<double>& diagonal_elMat, const ArrayT<int>& eqnos)
{
#if __option(extended_errorcheck)
	/* dimension check */
	if (diagonal_elMat.Length() != eqnos.Length()) throw ExceptionT::kSizeMismatch;
#endif

	/* assemble */
	for (int j = 0; j < eqnos.Length(); j++)
	{
		int eqno = eqnos[j] - 1;
		if (eqno > -1)
			fMatrix[eqno] += diagonal_elMat[j];
	}
}

/* compute the diagonal weighted residual norm - no check as
* to whether the matrix is factorized or not */
double CCSMatrixT::ResidualNorm(const dArrayT& result) const
{
	/* dimension check */
	if (result.Length() != fLocNumEQ) throw ExceptionT::kGeneralFail;

	double  norm = 0.0;
	const double* p = result.Pointer();

	for (int i = 0; i < fLocNumEQ; i++)
	{
		norm += (*p)*(*p)/fMatrix[fDiags[i]];
		p++;
	}

	return sqrt(norm);
}

/* compute the sum of the elements on the prescribed row/col */
double CCSMatrixT::AbsRowSum(int rownum) const
{
	/* dimension check */
	if (rownum < 0 || rownum >= fLocNumEQ) throw ExceptionT::kGeneralFail;

	register double rowsum = fabs( fMatrix[ fDiags[rownum] ] );
	
	int height;
				
	/* upper triangle  */
	if (rownum > 0)
		height = fDiags[rownum] - fDiags[rownum-1] - 1;
	else
		height = 0;
		
	if (height > 0)
	{
		double* pK = &fMatrix[fDiags[rownum-1] + 1];
		for (int i = 0; i < height; i++)
			rowsum += fabs(*pK++);
	}
			
	/* lower triangle */		
	for (int eqK = rownum + 1; eqK < fLocNumEQ; eqK++)
	{
		height = fDiags[eqK] - fDiags[eqK-1] - 1;
		int diff = height - (eqK - rownum);

		/* column height intercepts the row */		
		if (diff >= 0)
			rowsum += fabs(fMatrix[fDiags[eqK-1] + (diff + 1)]);
	}
	
	return rowsum;
}

/* return the value of p_i K_ij p_j */
double CCSMatrixT::pTKp(const dArrayT& p) const
{
	if (fIsFactorized)
	{
		dArrayT Up(fLocNumEQ);
		
		//unit diagonal contribution from U
		Up = p;
		
		//implement faster version later
		for (int j = 0; j < fLocNumEQ; j++)
			for (int k = j+1; k < fLocNumEQ; k++)
				Up[j] += (*this)(j,k)*p[k];
		
		double result = 0.0;
		
		for (int i = 0; i < fLocNumEQ; i++)
			result += Up[i]*Up[i]*fMatrix[fDiags[i]];
	
		return result;	
	}
	else /* not factorized */
	{
		dArrayT Kp(fLocNumEQ);
		Multx(p, Kp);
		return dArrayT::Dot(Kp,p);
	}
}

/* returns 1 if the factorized matrix contains a negative
* pivot.  Matrix MUST be factorized.  Otherwise function
* returns 0 */
int CCSMatrixT::HasNegativePivot(void) const
{
	if (!fIsFactorized)
		return 0;

	for (int i = 0; i < fLocNumEQ; i++)
		if (fMatrix[fDiags[i]] < 0.0)
			return 1;
	
	return 0;
}

/* assignment operator */
GlobalMatrixT& CCSMatrixT::operator=(const CCSMatrixT& RHS)
{
/* NOTE: when called by the copy constructor, GlobalMatrixT::operator=
 *       will be called before entering this subroutine; hence the
 *       inherited data will already be copied. This is my a more
 *       complicated test is needed to synch the memory below. */

	/* no copies of self */
	if (this != &RHS)
	{
		/* equation sets */
		fEqnos = RHS.fEqnos;
		fRaggedEqnos = RHS.fRaggedEqnos;
	
		/* sync memory */
		if (!fDiags || fLocNumEQ != RHS.fLocNumEQ)
		{
			/* free existing */
			delete[] fDiags;
				
			/* reallocate */
			fLocNumEQ = RHS.fLocNumEQ;
			fDiags = new int[fLocNumEQ];
			if (!fDiags) throw ExceptionT::kOutOfMemory;
		}

		/* copy bytes */	
		memcpy(fDiags, RHS.fDiags, sizeof(int)*fLocNumEQ);	
	
		/* sync memory */
		if (!fMatrix || fNumberOfTerms != RHS.fNumberOfTerms)
		{
			/* free existing */
			delete[] fMatrix;
				
			/* reallocate */
			fNumberOfTerms = RHS.fNumberOfTerms;
			fMatrix = new double[fNumberOfTerms];
			if (!fMatrix) throw ExceptionT::kOutOfMemory;
		}
	
		/* copy bytes */	
		memcpy(fMatrix, RHS.fMatrix, sizeof(double)*fNumberOfTerms);	

		/* inherited - do after since some dimensions are contained in
		 * base class */
		GlobalMatrixT::operator=(RHS);
	}
	
	return *this;
}

GlobalMatrixT& CCSMatrixT::operator=(const GlobalMatrixT& rhs)
{
#ifdef __NO_RTTI__
	cout << "\n CCSMatrixT::operator= : requires RTTI" << endl;
	throw ExceptionT::kGeneralFail;
#endif

	const CCSMatrixT* ccs = TB_DYNAMIC_CAST(const CCSMatrixT*, &rhs);	
	if (!ccs) ExceptionT::GeneralFail("CCSMatrixT::operator="," cast failed");

	return operator=(*ccs);
}

/* return a clone of self */
GlobalMatrixT* CCSMatrixT::Clone(void) const
{
	CCSMatrixT* new_mat = new CCSMatrixT(*this);
	return new_mat;
}

/* TESTING: write non-zero elements of matrix in Aztec readable
*          format */
void CCSMatrixT::WriteAztecFormat(ostream& out) const
{
	/* increase precision */
	int high_precision = 12;
	out.precision(high_precision);
	double* junk = NULL;
	int d_width = OutputWidth(out, junk);

	/* output number of equations */
	out << setw(kIntWidth) << fLocNumEQ << '\n';

	/* only write out "non-zero" entries */
	for (int i = 0; i < fLocNumEQ; i++)
	{
		for (int j = 0; j < fLocNumEQ; j++)
		{
			double val = (*this)(i,j);
			
			if (fabs(val) > kSmall)
			{
				out << setw(kIntWidth) << j;
				out << setw(d_width) << val << '\n';
			}	
		}

		out << setw(kIntWidth) << -1 << '\n';
	}

	/* restore stream precision */
	out.precision(kPrecision);
}

/* number scope and reordering */
GlobalMatrixT::EquationNumberScopeT CCSMatrixT::EquationNumberScope(void) const
{
	return kLocal;
}

bool CCSMatrixT::RenumberEquations(void) const { return true; }

/* find the smallest and largest diagonal value */
void CCSMatrixT::FindMinMaxPivot(double& min, double& max, double& abs_min, 
	double& abs_max) const
{
	if (fLocNumEQ == 0)  min = max = abs_min = abs_max = 0.0;
	else
	{
		abs_min = abs_max = min = max = fMatrix[fDiags[0]];
		for (int i = 1; i < fLocNumEQ; i++)
		{
			double& diag = fMatrix[fDiags[i]];

			/* absolute */
			if (diag < min)
				min = diag;
			else if (diag > max)
				max = diag;
				
			/* magnitude */	
			if (fabs(diag) < fabs(abs_min))
				abs_min = diag;
			else if (fabs(diag) > fabs(abs_max))
				abs_max = diag;
		}
	}
}

/**************************************************************************
* Protected
**************************************************************************/

/* element accessor */
double CCSMatrixT::operator()(int row, int col) const
{
	if (row > col) /* element in lower triangle */
		return (*this)(col,row);
	else
	{
		/* range checks */
		if (row < 0 || row >= fLocNumEQ) throw ExceptionT::kGeneralFail;
		if (col < 0 || col >= fLocNumEQ) throw ExceptionT::kGeneralFail;
	
		if (row == col) /* element on the diagonal */
			return fMatrix[fDiags[col]];
		else
		{
			int colht = ColumnHeight(col);
			int hrow  = col-row;
					
			if (hrow > colht) /* element above the skyline */
				return 0.0;
			else
				return fMatrix[fDiags[col] - hrow];
		}
	}	
}

namespace Tahoe {

ostream& operator<<(ostream& out, const CCSMatrixT& matrix)
{
	double* junk = NULL;
	int d_width = OutputWidth(out, junk);

	for (int col = 0; col < matrix.fLocNumEQ; col++)
	{
		for (int row = 0; row < matrix.fLocNumEQ; row++)
			out << setw(d_width) << matrix(row,col);
	
		out << '\n';
	}

	return out;
}

}

/* solution routines */
void CCSMatrixT::Factorize(void)
{			
	int		j, jj, jjlast, jcolht;
	int		i, ii, ij, icolht, istart, iilast;
	int		jm1, jlength, jtemp, length;
	double	temp;
	int     fail = 0;

	/* factorize */
	jj = -1;		/* -1 for C arrays starting at zero */
	
for (j = 0; j < fLocNumEQ; j++)
{
	jjlast = jj;
	jj     = fDiags[j];
	jcolht = jj - jjlast;	

	if (jcolht > 2)
	{
			/* for column j and i <= j-1, replace a[i,j] with d[i,i]*u[i,j] */
	istart = j - jcolht + 2;
	jm1    = j - 1;
	ij     = jjlast + 2;
	ii     = fDiags[istart-1];

			for (i = istart; i <= jm1; i++)
			{
		iilast  = ii;
		ii      = fDiags[i];
		icolht  = ii - iilast;
		jlength = i - istart + 1;
		length  = Min(icolht-1, jlength);
		
		if (length > 0)
			fMatrix[ij] -= Dot(&fMatrix[ii-length],
			                   &fMatrix[ij-length],
			                   length);
		ij++;
			}
	}

if (jcolht >= 2)
{
			/* for column j and i <= j-1, replace a[i,j] with u[i,j]	*/
			/* and replace a[j,j] with d[j,j]. 							*/
	         jtemp = j - jj;
	
	         for (ij = jjlast+1; ij <= jj-1; ij++)
	         {
		ii = fDiags[jtemp + ij];
		
			/* warning: the following calculations are skipped 			*/
			/* if a(ii) equals zero										*/
			
				if (fMatrix[ii] != 0.0)
				{
		temp  = fMatrix[ij];
		fMatrix[ij] = temp/fMatrix[ii];
		fMatrix[jj]-= temp*fMatrix[ij];
		}
				else
				    fail = 1;
			}
		}
	}

	if (fail)
	{
		cout << "\n CCSMatrixT::Factorize: factorization is approximate due to zero";
		cout << " values on the diagonal" << endl;
	}
}

void CCSMatrixT::BackSubstitute(dArrayT& result)
{

	int		j, jj, jjlast, jjnext, jcolht;
	int		i, istart, jtemp;
	double	ajj, bj;
	double* resultPtr = result.Pointer();
	int     fail = 0;

	/* check */
	if (!fIsFactorized) throw ExceptionT::kGeneralFail;
		
	/* forward reduction */
	jj = -1;
	
	for (j = 0; j < fLocNumEQ; j++)
	{
		jjlast = jj;
		jj 	   = fDiags[j];
		jcolht = jj - jjlast;
		
		if (jcolht > 1)
			result[j] -= Dot(fMatrix + jjlast + 1,
			                 resultPtr + j - jcolht + 1,
			                 jcolht - 1);
	}
	
	/* diagonal scaling */
	for (j = 0; j < fLocNumEQ; j++)
	{
		ajj = fMatrix[fDiags[j]];
		
		/* warning: diagonal scaling is not performed if ajj equals zero */
		if (ajj != 0.0)
			result[j] = result[j]/ajj;
		else
		    fail = 1;
	}
	
	/* back substitution */
	if (fLocNumEQ > 1)
	{
		jjnext = fDiags[fLocNumEQ - 1];
		
		for (j = fLocNumEQ-1; j > 0; j--)
		{
			jj     = jjnext;
			jjnext = fDiags[j-1];
			jcolht = jj - jjnext;
			
			if (jcolht > 1)
			{
				bj = result[j];
				istart = j - jcolht + 1;
				jtemp  = jjnext + 1;

				double* presult = resultPtr + istart;
				double* pmatrix = fMatrix + jtemp;
				for (i = istart; i < j; i++)
					*presult++ -= (*pmatrix++)*bj;				
			}
		}
	}

	if (fail)
	{
		cout << "\n CCSMatrixT::BackSubstitute: solution is approximate due to zero";
		cout << " values on the diagonal" << endl;
	}
}

/* check functions */
void CCSMatrixT::PrintZeroPivots(void) const
{
	if (fCheckCode != GlobalMatrixT::kZeroPivots || fLocNumEQ == 0) return;
	int d_width = OutputWidth(fOut, fMatrix);

	/* pivot extrema */
	double min, max, abs_min, abs_max;
	FindMinMaxPivot(min, max, abs_min, abs_max);
	fOut << "\n Matrix pivots:\n"
	     <<   "     min = " << setw(d_width) << min << '\n' 
	     <<   "     max = " << setw(d_width) << max << '\n' 
	     <<   "   |min| = " << setw(d_width) << abs_min << '\n' 
	     <<   "   |max| = " << setw(d_width) << abs_max << '\n';

	/* write zero or negative pivots */
	int firstline = 1;
	for (int i = 0; i < fLocNumEQ; i++)
	{
		double pivot = fMatrix[fDiags[i]];
		
		if (pivot < kSmall)
		{
			if (firstline)
			{
				fOut << "\n Zero or negative pivots:\n\n";
				firstline = 0;
			}
		
			fOut << setw(kIntWidth) << i + 1;
			fOut << setw(d_width) << pivot << '\n';
		}
	}
	
	if (!firstline)
		fOut << '\n';
}

void CCSMatrixT::PrintAllPivots(void) const
{
	if (fCheckCode != GlobalMatrixT::kAllPivots) return;
	int d_width = OutputWidth(fOut, fMatrix);

	fOut << "\nAll pivots:\n\n";
	
	for (int i = 0; i < fLocNumEQ; i++)
	{
		fOut << setw(kIntWidth) << i + 1;
		fOut << setw(d_width) << fMatrix[fDiags[i]] << '\n';
	}
	fOut << '\n';
}

void CCSMatrixT::PrintLHS(bool force) const
{
	if (!force && fCheckCode != GlobalMatrixT::kPrintLHS) return;
	
	fOut << "\nLHS matrix:\n\n";
	fOut << (*this) << "\n\n";
	
//TEMP - write to Aztec format	
//cout << "\n Writing Aztec output file: .data" << endl;
//ofstream az_out(".data");
//az_out.setf(ios::showpoint);
//az_out.setf(ios::right, ios::adjustfield);
//az_out.setf(ios::scientific, ios::floatfield);
//WriteAztecFormat(az_out);
}


/* matrix-vector product */
bool CCSMatrixT::Multx(const dArrayT& d, dArrayT& Kd) const
{
	/* consistency check */
	if (fIsFactorized) return false;

	/* dimension checks */
	if ( d.Length() != Kd.Length() ||
	     d.Length() != fLocNumEQ ) throw ExceptionT::kGeneralFail;

	Kd = 0.0;
	
	/* diagonal */
	for (int j = 0; j < fLocNumEQ; j++)
		Kd[j] += d[j]*fMatrix[ fDiags[j] ];
		
	/* upper triangle */
	int dexd = 1;
	for (int i = 1; i < fLocNumEQ; i++)
	{
		int height = fDiags[i] - fDiags[i-1] - 1;
		
		if (height > 0)
		{
			int dexK = fDiags[i] - height;
			int eqK  = i - height;
			
			while (eqK < i)
			{
				Kd[eqK] += d[dexd]*fMatrix[dexK];
				dexK++;
				eqK++;
			}
		}
		dexd++;
	}
			
	/* lower triangle */		
	for (int eqK = 1; eqK < fLocNumEQ; eqK++)
	{
		int height = fDiags[eqK] - fDiags[eqK-1] - 1;
		
		if (height > 0)
		{
			int dexK = fDiags[eqK] - height;
			int dexd = eqK - height;
			
			while (dexd < eqK)
			{
				Kd[eqK] += d[dexd]*fMatrix[dexK];
				dexK++;
				dexd++;
			}
		}
	}
	
	/* OK */
	return true;
}									

/* return the values along the diagonal of the matrix */
bool CCSMatrixT::CopyDiagonal(dArrayT& diags) const
{
	/* cannot be factorized */
	if (fIsFactorized)
		return false;
	else
	{
		/* dimension */
		diags.Dimension(fLocNumEQ);

		/* collect */
		for (int j = 0; j < fLocNumEQ; j++)
			diags[j] = fMatrix[fDiags[j]];
	
		return true;		
	}
}

/**************************************************************************
* Private
**************************************************************************/

/* (re-) compute the matrix structure and return the bandwidth
* and mean bandwidth */
void CCSMatrixT::ComputeSize(int& num_nonzero, int& mean_bandwidth, int& bandwidth)
{
	/* check */
	if (fLocNumEQ > 0 && !fDiags) throw ExceptionT::kGeneralFail;

	/* clear diags/columns heights */
	for (int i = 0; i < fLocNumEQ; i++)
		fDiags[i] = 0;
		
	/* compute column heights */
	const iArray2DT* peq;
	fEqnos.Top();
	while (fEqnos.Next(peq))
		SetColumnHeights(*peq);		

	const RaggedArray2DT<int>* prageq;
	fRaggedEqnos.Top();
	while (fRaggedEqnos.Next(prageq))
		SetColumnHeights(*prageq);		

	/* set diagonal positions */
	mean_bandwidth = 0;
	bandwidth     = 0;
	if (fLocNumEQ > 0)
	{
		fDiags[0] = 0;
		if (fLocNumEQ > 1)
			for (int eq = 1; eq < fLocNumEQ; eq++)
			{
				/* find max column height */
				bandwidth = (fDiags[eq] > bandwidth) ? fDiags[eq] : bandwidth;
				
				fDiags[eq] += fDiags[eq-1] + 1;
			}			
		num_nonzero = fDiags[fLocNumEQ-1] + 1;

		/* final dimensions */
		mean_bandwidth = int(ceil(double(num_nonzero)/fLocNumEQ));
		bandwidth += 1;
	}
}

/* computes the column heights for the given equation list */
void CCSMatrixT::SetColumnHeights(const iArray2DT& eqnos)
{
	int nel = eqnos.MajorDim(); /* number of elements */
	int nee = eqnos.MinorDim(); /* number of element equations */

	for (int j = 0; j < nel; j++)
	{
		const int* eleqnos = eqnos(j);
		int min = fLocNumEQ;
	
		/* find the smallest eqno > 0 */
		for (int k = 0; k < nee; k++)
		{
			int eq = eleqnos[k];
		
			if (eq > 0 && eq < min)
				min = eq;
		}
	
		/* set column height */
		for (int i = 0; i < nee; i++)
		{
			int eq = eleqnos[i];
		
			if (eq > 0)
			{
				int height = eq - min; /* this is the number of elements */
									   /* ABOVE the diagonal */
			
				int& currheight = fDiags[--eq];
				
				if (height > currheight)
					currheight = height;
			}
		}
	}
}

/* computes the column heights for the given equation list */
void CCSMatrixT::SetColumnHeights(const RaggedArray2DT<int>& eqnos)
{
	/* loop over elements */
	int nel = eqnos.MajorDim();
	for (int j = 0; j < nel; j++)
	{
		int nee = eqnos.MinorDim(j);
		const int* eleqnos = eqnos(j);
	
		/* find the smallest eqno > 0 */
		int min = fLocNumEQ;
		for (int k = 0; k < nee; k++)
		{
			int eq = eleqnos[k];
		
			if (eq > 0 && eq < min)
				min = eq;
		}
	
		/* set column height */
		for (int i = 0; i < nee; i++)
		{
			int eq = eleqnos[i];
		
			if (eq > 0)
			{
				int height = eq - min; /* this is the number of elements */
									   /* ABOVE the diagonal */
			
				int& currheight = fDiags[--eq];
				
				if (height > currheight)
					currheight = height;
			}
		}
	}
}

/* returns number of non-zero elements */
int CCSMatrixT::NumberOfFilled(int printRCV)
{
	/* make all 0.0 */
	Clear();

	const iArray2DT* peq;
	fEqnos.Top();
	while (fEqnos.Next(peq))
		FillWithOnes(*peq);		

	const RaggedArray2DT<int>* prageq;
	fRaggedEqnos.Top();
	while (fRaggedEqnos.Next(prageq))
		FillWithOnes(*prageq);		

	/* count number non-zero */
	int count = 0;
	double* p = fMatrix;
	for (int i = 0; i < fNumberOfTerms; i++)
		if (*p++ > 0.0)
			count++;

	/* output {row, col, val} data */
	if (printRCV)
	{
		ofstream rows("rows.dat");
		ofstream cols("cols.dat");
	
		/* first column */
		rows << 1 << '\n';
		cols << 1 << '\n';
	
		/* all other columns */
		for (int i = 1; i < fLocNumEQ; i++)
		{
			int height = fDiags[i] - fDiags[i-1];

			/* output indices of nonzero entries */
			double* pcol = fMatrix + fDiags[i];
			for (int j = 0; j < height; j++)
				if (*pcol-- > 0.0)
				{
					rows << i-j+1 << '\n';
					cols << i+1   << '\n';				
				}
		}
	}

	return count;
}

/* place 1.0 in elements that will be filled */
void CCSMatrixT::FillWithOnes(const iArray2DT& eqnos)
{
	int nel = eqnos.MajorDim(); /* number of elements */
	int nee = eqnos.MinorDim(); /* number of element equations */

	/* make element stiffness filled with ones */
	ElementMatrixT elstiff(nee, ElementMatrixT::kSymmetric);
	elstiff = 1.0;

	iArrayT localeqnos;
	for (int i = 0; i < nel; i++)
	{
		eqnos.RowAlias(i,localeqnos);
		Assemble(elstiff, localeqnos);
	}
}

/* place 1.0 in elements that will be filled */
void CCSMatrixT::FillWithOnes(const RaggedArray2DT<int>& eqnos)
{
	/* number of elements */
	int nel = eqnos.MajorDim();

	/* make element stiffness filled with ones */
	int maxlength = eqnos.MaxMinorDim();
	dArrayT space(maxlength*maxlength);
	space = 1.0;

	ElementMatrixT elstiff(ElementMatrixT::kSymmetric);

	iArrayT localeqnos;
	for (int i = 0; i < nel; i++)
	{
		int nee = eqnos.MinorDim(i);
		elstiff.Set(nee, nee, space.Pointer());
	
		eqnos.RowAlias(i,localeqnos);
		Assemble(elstiff, localeqnos);
	}
}
