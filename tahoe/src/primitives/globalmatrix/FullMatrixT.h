/* $Id: FullMatrixT.h,v 1.9 2002-10-20 22:49:33 paklein Exp $ */
/* created: paklein (03/07/1998) */

#ifndef _FULL_MATRIX_T_H_
#define _FULL_MATRIX_T_H_

/* base class */
#include "GlobalMatrixT.h"

/* direct members */
#include "LAdMatrixT.h"

namespace Tahoe {

/** full matrix */
class FullMatrixT: public GlobalMatrixT
{
public:

	/** constructor */
	FullMatrixT(ostream& out, int check_code);

	/** copy constructor */
	FullMatrixT(const FullMatrixT& source);
		
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

	/* strong manipulation functions */
	//TEMP should be pure virtual, but no time to update others
	//     so just throw ExceptionT::xception for now
	virtual void OverWrite(const ElementMatrixT& elMat, const nArrayT<int>& eqnos);
	virtual void Disassemble(dMatrixT& matrix, const nArrayT<int>& eqnos) const;
	virtual void DisassembleDiagonal(dArrayT& diagonals, const nArrayT<int>& eqnos) const;

	/* number scope and reordering */
	virtual EquationNumberScopeT EquationNumberScope(void) const;
	virtual bool RenumberEquations(void) const;

	/** assignment operator */
	virtual GlobalMatrixT& operator=(const FullMatrixT& rhs);

	/** assignment operator */
	virtual GlobalMatrixT& operator=(const GlobalMatrixT& rhs);
	
	/** return a clone of self. Caller is responsible for disposing of the matrix */
	virtual GlobalMatrixT* Clone(void) const;

	/** matrix-vector product. Cannot be called after the matrix is factorized. */
	virtual bool Multx(const dArrayT& x, dArrayT& b) const;

	/** Tranpose[matrix]-vector product. Cannot be called after the matrix is 
	 * factorized. */
	virtual bool MultTx(const dArrayT& x, dArrayT& b) const;
	
protected:

	/* precondition matrix */
	virtual void Factorize(void);
	
	/* solution driver */
	virtual void BackSubstitute(dArrayT& result);

	/* rank check functions */
	virtual void PrintAllPivots(void) const;
	virtual void PrintZeroPivots(void) const;
	virtual void PrintLHS(void) const;

protected:

	/* the matrix */
	LAdMatrixT fMatrix;
};

} // namespace Tahoe 
#endif /* _FULL_MATRIX_T_H_ */
