/* $Id: DiagonalMatrixT.h,v 1.7 2002-07-02 19:56:45 cjkimme Exp $ */
/* created: paklein (03/23/1997) */

#ifndef _DIAGONAL_MATRIX_H_
#define _DIAGONAL_MATRIX_H_

/* base class */
#include "GlobalMatrixT.h"

/* direct members */
#include "dArrayT.h"

/** diagonal matrix */

namespace Tahoe {

class DiagonalMatrixT: public GlobalMatrixT
{
public:

	/** enum to signal how to assemble non-diagonal contributions to the matrix */
	enum AssemblyModeT {kNoAssembly = 0, /**< do not assemble, throw exception */ 
	                    kDiagOnly   = 1, /**< assemble the diagonal values only */
                        kAbsRowSum  = 2  /**< assemble the L1 norm of the row */};

	/** constructors */
	DiagonalMatrixT(ostream& out, int check_code, AssemblyModeT mode);

	/** copy constructor */
	DiagonalMatrixT(const DiagonalMatrixT& source);

	/** DiagonalMatrixT::Solve does preserve the data in the matrix */
	virtual bool SolvePreservesData(void) const { return true; };	  

	/* set assemble mode */
	void SetAssemblyMode(AssemblyModeT mode);

	/* set the internal matrix structure.
	 * NOTE: do not call Initialize() equation topology has been set
	 * with AddEquationSet() for all equation sets */
	virtual void Initialize(int tot_num_eq, int loc_num_eq, int start_eq);
	
	/* set all matrix values to 0.0 */
	virtual void Clear(void);
		
	/* add element group equations to the overall topology.
	 * NOTE: assembly positions (equation numbers) = 1...fDimension
	 * equations can be of fixed size (iArray2DT) or
	 * variable length (RaggedArray2DT) */
	virtual void AddEquationSet(const iArray2DT& eqset);
	virtual void AddEquationSet(const RaggedArray2DT<int>& eqset);
	
	/* assemble the element contribution into the LHS matrix - assumes
	 * that elMat is square (n x n) and that eqnos is also length n.
	 * NOTE: assembly positions (equation numbers) = 1...fDimension */
	virtual void Assemble(const ElementMatrixT& elMat, const nArrayT<int>& eqnos);
	virtual void Assemble(const ElementMatrixT& elMat, const nArrayT<int>& row_eqnos,
		const nArrayT<int>& col_eqnos);

	/* fetch values */
	virtual void DisassembleDiagonal(dArrayT& diagonals, const nArrayT<int>& eqnos) const;

	/* access to the data */
	dArrayT& TheMatrix(void);

	/* number scope and reordering */
	virtual EquationNumberScopeT EquationNumberScope(void) const;
	virtual bool RenumberEquations(void) const;

	/** assignment operator */
	virtual GlobalMatrixT& operator=(const DiagonalMatrixT& rhs);

	/** assignment operator */
	virtual GlobalMatrixT& operator=(const GlobalMatrixT& rhs);
	
	/** return a clone of self. Caller is responsible for disposing of the matrix */
	virtual GlobalMatrixT* Clone(void) const;

	/** matrix-vector product. OK to call either before or after the matrix is
	 * factorized */
	virtual bool Multx(const dArrayT& x, dArrayT& b) const;

	/** Tranpose[matrix]-vector product. OK to call either before or after the matrix 
	 * is factorized */
	virtual bool MultTx(const dArrayT& x, dArrayT& b) const;
	
protected:

	/* precondition matrix */
	virtual void Factorize(void);
	
	/* solution driver */
	virtual void BackSubstitute(dArrayT& result);

	/* check functions */
	virtual void PrintAllPivots(void) const;
	virtual void PrintZeroPivots(void) const;
	virtual void PrintLHS(void) const;
	
private:

	dArrayT	fMatrix;
	
	/* mode flag */
	AssemblyModeT fMode;
};

/* inlines */

/* access to the data */
inline dArrayT& DiagonalMatrixT::TheMatrix(void)
{
	return fMatrix;
}

/* Tranpose[matrix]-vector product */
inline bool DiagonalMatrixT::MultTx(const dArrayT& x, dArrayT& b) const 
{ 
	return DiagonalMatrixT::Multx(x, b);
}; 

} // namespace Tahoe 
#endif /* _DIAGONAL_MATRIX_H_ */
