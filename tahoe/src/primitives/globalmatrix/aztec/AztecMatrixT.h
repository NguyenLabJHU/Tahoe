/* $Id: AztecMatrixT.h,v 1.8 2002-11-30 16:31:05 paklein Exp $ */
/* created: paklein (08/10/1998) */
#ifndef _AZTEC_MATRIX_T_H_
#define _AZTEC_MATRIX_T_H_

/* base classes */
#include "GlobalMatrixT.h"

/* library support option */
#ifdef __AZTEC__

/* direct members */
#include "AutoArrayT.h"
#include "dMatrixT.h"

namespace Tahoe {

/* forward declarations */
class Aztec_fe;
class ifstreamT;

/** interface for Aztec linear solver library */
class AztecMatrixT: public GlobalMatrixT
{
public:

	/* constuctor */
	AztecMatrixT(ifstreamT& in, ostream& out, int check_code);

	/** copy constructor */
	AztecMatrixT(const AztecMatrixT& source);
	
	/* destuctor */
	virtual ~AztecMatrixT(void);

	/* set matrix structure and allocate space.
	 * NOTE: do not call Initialize until all equations sets have been
	 * registered with SetStructure */
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
	 *
	 * NOTE: assembly positions (equation numbers) = 1...fNumEQ */
	virtual void Assemble(const ElementMatrixT& elMat, const nArrayT<int>& eqnos);
	virtual void Assemble(const ElementMatrixT& elMat, const nArrayT<int>& row_eqnos,
		const nArrayT<int>& col_eqnos);
	virtual void Assemble(const nArrayT<double>& diagonal_elMat, const nArrayT<int>& eqnos);

	/* number scope and reordering */
	virtual EquationNumberScopeT EquationNumberScope(void) const;
	virtual bool RenumberEquations(void) const;

	/** return the form of the matrix */
	virtual GlobalT::SystemTypeT MatrixType(void) const { return GlobalT::kNonSymmetric; };

	/** assignment operator */
	virtual GlobalMatrixT& operator=(const AztecMatrixT& rhs);

	/** assignment operator */
	virtual GlobalMatrixT& operator=(const GlobalMatrixT& rhs);
	
	/** return a clone of self. Caller is responsible for disposing of the matrix */
	virtual GlobalMatrixT* Clone(void) const;

protected:

	/* precondition matrix */
	virtual void Factorize(void);
	
	/* determine new search direction and put the results in result */
	virtual void BackSubstitute(dArrayT& result);

	/* rank check functions */
	virtual void PrintAllPivots(void) const;
	virtual void PrintZeroPivots(void) const;
	virtual void PrintLHS(void) const;

private:

	/* input stream */
	ifstreamT& fInput;
	
	/* C++ interface to Aztec 1.0 */
	Aztec_fe* fAztec;
	
	/* for assembly operations */
	AutoArrayT<int> fRowDexVec, fColDexVec;
	AutoArrayT<double> fValVec;
	dMatrixT fValMat;
};

/* library support options */
} // namespace Tahoe 
#endif /* __AZTEC__ */
#endif /* _AZTEC_MATRIX_T_H_ */
