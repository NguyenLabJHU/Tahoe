/* $Id: CCNSMatrixT.h,v 1.7.4.1 2002-10-17 04:47:07 paklein Exp $ */
/* created: paklein (03/04/1998)                                          */
/* This is the interface for a non-symmetric matrix stored in             */
/* Compact Column form.                                                   */
/* To initialize:                                                         */
/* 			(1) call constructor with system dimension dim                */
/* 			(2) set the ColumnHeight for column = 0...dim-1               */
/* 			(3) call Initialize() to allocate space for the               */
/* matrix and set the diagonal position array.                            */
/* Note: assembly positions (equation numbers) = 1...fNumEQ               */
/* Note: source code for factorization and back substitution provided     */
/* by Gonzalo Feijoo.                                                     */
/* SolNonSymSkyLine - March, 1990                                         */
/* SOLves NON-SYMmetric system stored in SKY-LINE form.                   */
/* Gonzalo Raul Feijoo                                                    */
/* Performs solution of linear system with coefficients                   */
/* stored in descending sky-line form.                                    */
/* This function decomposes the matrix, which coefficients are stored     */
/* in KS, KI, KD vectors in LU form. Then it solves (if asked) the lower  */
/* triangular system Lu' = F and the upper triangular system U u = u'.    */
/* Entries:                                                               */
/* KS, KI, KD      system matrix: upper, lower and diagonal part          */
/* F               independent term                                       */
/* maxa            contains position in vector KS or KI of first non-zero */
/* coefficient of column or line.                                         */
/* neq             number of equations in the system                      */
/* solve           if 1 solves the system                                 */
/* eps             the 0.0                                                */
/* Output:                                                                */
/* KS, KI, KD      matrix decomposed in LU form                           */
/* u               solution of linear system (if solve == 1)              */
/* Errors:         returns 1 if there were no problems, 0 otherwise.      */
/* ATTENTION:      vector F is modified by the algorithm if solve == 1.   */

#ifndef _CCNSMATRIX_T_H_
#define _CCNSMATRIX_T_H_

/* project headers */
#include "Environment.h"
#include "ExceptionT.h"

/* base class */
#include "GlobalMatrixT.h"

/* direct members */
#include "LinkedListT.h"

namespace Tahoe {

class CCNSMatrixT: public GlobalMatrixT
{
public:

	/** constructor */
	CCNSMatrixT(ostream& out, int check_code);

	/** copy constructor */
	CCNSMatrixT(const CCNSMatrixT& source);

	/* destructor */	
	virtual ~CCNSMatrixT(void);
	
	/* set the internal matrix structure.
	 * NOTE: do not call Initialize() equation topology has been set
	 * with AddEquationSet() for all equation sets */
	virtual void Initialize(int tot_num_eq, int loc_num_eq, int start_eq);
	
	/* set all matrix values to 0.0 */
	virtual void Clear(void);

	/* add element group equations to the overall topology.
	 * NOTE: assembly positions (equation numbers) = 1...fNumEQ
	 * equations can be of fixed size (iArray2DT) or
	 * variable length (RaggedArray2DT) */
	virtual void AddEquationSet(const iArray2DT& eqset);
	virtual void AddEquationSet(const RaggedArray2DT<int>& eqset);
		
	/* assemble the element contribution into the LHS matrix - assumes
	 * that elMat is square (n x n) and that eqnos is also length n.
	 * NOTE: assembly positions (equation numbers) = 1...fNumEQ */
	virtual void Assemble(const ElementMatrixT& elMat, const nArrayT<int>& eqnos);
	virtual void Assemble(const ElementMatrixT& elMat, const nArrayT<int>& row_eqnos,
		const nArrayT<int>& col_eqnos);
	
	/* returns 1 if the factorized matrix contains a negative
	 * pivot.  Matrix MUST be factorized.  Otherwise function
	 * returns 0 */
	int HasNegativePivot(void) const;

	/* element accessor - READ ONLY */
	double Element(int row, int col) const;			

	/* number scope and reordering */
	virtual EquationNumberScopeT EquationNumberScope(void) const;
	virtual bool RenumberEquations(void) const;

	/* find the smallest and largest diagonal value */
	void FindMinMaxPivot(double& min, double& max, double& abs_min, double& abs_max) const;

	/** assignment operator */
	virtual GlobalMatrixT& operator=(const CCNSMatrixT& rhs);

	/** assignment operator */
	virtual GlobalMatrixT& operator=(const GlobalMatrixT& rhs);

	/** return a clone of self. Caller is responsible for disposing of the matrix */
	virtual GlobalMatrixT* Clone(void) const;

	/** return the values along the diagonal of the matrix. Derived classes
	 * must reimplement this function to extrat the diagonals from the
	 * matrix-specific storage schemes.
	 * \param diags returns with the diagonals of the matrix if the function
	 *        is supported. Otherwise is left unchanged.
	 * \return true if the diagonal values where collected successfully */
	virtual bool CopyDiagonal(dArrayT& diags) const;

protected:
	
	/* output operator */
	friend ostream& operator<<(ostream& out, const CCNSMatrixT& matrix);	

	/* precondition matrix */
	virtual void Factorize(void);
	
	/* solution driver */
	virtual void BackSubstitute(dArrayT& result);

	/* check functions */
	virtual void PrintAllPivots(void) const;
	virtual void PrintZeroPivots(void) const;
	virtual void PrintLHS(void) const;

	/* test if {row,col} is within the skyline */
	int InSkyline(int row, int col) const;

	/* elements beyond the diagonal (above and to the left) */
	int BandWidth(int eqnum) const;

	/* element accessor - for assembly - exception to access out of skyline */
	double& operator()(int row, int col) const;			
	
private:

	/* (re-) compute the matrix structure and return the bandwidth
	 * and mean bandwidth */
	void ComputeSize(int& num_nonzero, int& mean_bandwidth, int& bandwidth);

	/* computes the skyline for the given equation list */
	void SetSkylineHeights(const iArray2DT& eqnos);
	void SetSkylineHeights(const RaggedArray2DT<int>& eqnos);

	/* Returns number of non-zero elements */
	int  NumberOfFilled(void);
	void FillWithOnes(const iArray2DT& eqnos);
	void FillWithOnes(const RaggedArray2DT<int>& eqnos);

	/* factorization routine (GRF) */
	void SolNonSymSysSkyLine( double* KS, double* KI, double* KD,
		int* maxa, int neq);

	/* back substitution routines (GRF) */
	/* solves L u'= F -> F := u' */
	void solvLT(double* KI, double* F, int* maxa, int neq);
	/* solves U u = u' = F       */
	void solvUT(double* KS, double* KD, double* u, double* F, int* maxa, int neq);

protected:

	/* equations sets */
	LinkedListT<const iArray2DT*> fEqnos;
	LinkedListT<const RaggedArray2DT<int>*> fRaggedEqnos;

	/* skyline */
	int* famax; // length fNumEQ+1

	/* matrix components - pointers to fMatrix */
	double* fKU; // upper triangle
	double*	fKL; // lower triangle
	double*	fKD; // diagonal

	/* storage */   	
	int		fNumberOfTerms;
	double* fMatrix;

private:

	/* workspace */
	double* fu;
};

/* utility */
inline int CCNSMatrixT::BandWidth(int eqnum) const
{
#if __option (extended_errorcheck)
	if (eqnum < 0 || eqnum >= fLocNumEQ) throw ExceptionT::kGeneralFail;
#endif
	
	return (eqnum == 0) ? 0 : famax[eqnum+1] - famax[eqnum];
}

} // namespace Tahoe 
#endif /* _CCNSMATRIX_T_H_ */
