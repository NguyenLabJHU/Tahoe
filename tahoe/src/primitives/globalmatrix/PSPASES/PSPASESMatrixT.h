/* $Id: PSPASESMatrixT.h,v 1.3 2004-03-14 02:27:48 paklein Exp $ */
#ifndef _PSPASES_MATRIX_T_H_
#define _PSPASES_MATRIX_T_H_

/* base class */
#include "GlobalMatrixT.h"

//TEMP
#define __PSPASES__

/* library support options */
#ifdef __PSPASES__

#ifndef __TAHOE_MPI__
#error "PSPASESMatrixT requires __TAHOE_MPI__"
#endif

/* direct members */
#include "AutoArrayT.h"
#include "iArrayT.h"
#include "dArrayT.h"
#include "iArray2DT.h"
#include "nVariArray2DT.h"

namespace Tahoe {

/* forward declarations */
class MSRBuilderT;
class CommunicatorT;

/** interface to PSPASES (1.0.3) sparse, direct linear solver. See 
 * http://www-users.cs.umn.edu/~mjoshi/pspases for more information.
 * The solver must be used with \e p number of processes, where \e p
 * is a power of 2 equal to or greater than 2. The solver is limited to
 * symmetric matricies, so all element matricies are symmetrized during
 * assembly.
 */
class PSPASESMatrixT: public GlobalMatrixT
{
public:

	/** constuctor */
	PSPASESMatrixT(ostream& out, int check_code, CommunicatorT& comm);

	/** copy constructor */
	PSPASESMatrixT(const PSPASESMatrixT& source);

	/** destructor */
	virtual ~PSPASESMatrixT(void);

	/** set the internal matrix structure */
	virtual void Initialize(int tot_num_eq, int loc_num_eq, int start_eq);

	/** \name set matrix structure
	 * active equations are eq > 0 */
	/*@{*/
	void AddEquationSet(const iArray2DT& eqnos);
	void AddEquationSet(const RaggedArray2DT<int>& eqnos);
	/*@}*/

	/** \name assembly operators
	 * active equations are eq > 0 */	
	/*@{*/
	virtual void Assemble(const ElementMatrixT& elMat, const ArrayT<int>& eqnos);
	virtual void Assemble(const ElementMatrixT& elMat, const ArrayT<int>& row_eqnos,
		const ArrayT<int>& col_eqnos);
	virtual void Assemble(const nArrayT<double>& diagonal_elMat, const ArrayT<int>& eqnos);
	/*@}*/

	/** PSPASESMatrixT::Solve does preserve the data in the matrix */
	virtual bool SolvePreservesData(void) const { return true; };	  

	/** return the form of the matrix */
	virtual GlobalT::SystemTypeT MatrixType(void) const { return GlobalT::kSymmetric; };

	/** set all matrix values to 0.0 */
	virtual void Clear(void);

	/** number scope */
	virtual EquationNumberScopeT EquationNumberScope(void) const { return kGlobal; };

	/** matrix prefers optimal ordering */
	virtual bool RenumberEquations(void) const { return false; };	
	
	/** \name assignment and duplication */
	/*@{*/
	/** assignment operator */
	virtual GlobalMatrixT& operator=(const PSPASESMatrixT& rhs);

	/** assignment operator */
	virtual GlobalMatrixT& operator=(const GlobalMatrixT& rhs);
	
	/** return a clone of self. Caller is responsible for disposing of the matrix */
	virtual GlobalMatrixT* Clone(void) const;
	/*@}*/

protected:

	/** \name check functions */
	/*@{*/
	virtual void PrintAllPivots(void) const;
	virtual void PrintZeroPivots(void) const;
	virtual void PrintLHS(bool force = false) const;
	/*@}*/

	/** precondition matrix */
	virtual void Factorize(void);
	
	/** determine new search direction and put the results in result */
	virtual void BackSubstitute(dArrayT& result);

private:
	
	/** element accessor. Returns a pointer to the given element in the matrix
	 * or NULL if the element is not in the set of non-zero elements. The search
	 * for columns is performed through recursive bisection of the indicies of
	 * columns with non-zero elements in the specified row. */
	double* operator()(int row, int col);

protected:

	/** MP environment */
	CommunicatorT& fComm;	

	/** matrix structure builder */
	MSRBuilderT* fBuilder;

	/** runtime flag */
	int fIsSymFactorized;

	/** \name matrix structure and data */
	/*@{*/
	/** global list of offsets of active rows from the first row (1...) */
	iArrayT frowdist;

	/** size and location (1...) of colums indicies in PSPASESMatrixT::fainds for each
	 * for in PSPASESMatrixT::frowdist belonging to this processor. Indicies (i,j) in this
	 * array correspond to indicies (j,i) of the arrays shown in the documentation because
	 * the information (j,i) needs to be column-major. */
	iArray2DT faptrs;
	
	/** memory manager for PSPASESMatrixT::faptrs */	
	nVariArray2DT<int> faptrs_man;

	/** column indicies of non-zero values */
	AutoArrayT<int> fainds;

	/** matrix values */
	AutoArrayT<double> favals;
	
	/** global permutation vector */
	AutoArrayT<int> forder;
	
	/** leaf subtree and supernode sizes */
	iArrayT fsizes;
	
	/** space for passing in RHS vector */
	AutoArrayT<double> fb;
	/*@}*/

	/** \name options arrays */
	/*@{*/
	/** PSPACEO options */
	iArrayT fioptions_PSPACEO;

	/** PSPACEY options */
	iArrayT fioptions_PSPACEY;

	/** PSPACEY double options */
	dArrayT fdoptions_PSPACEY;

	/** DPSPACET options */
	iArrayT fioptions_DPSPACET;
	/*@}*/
	
	/** Y-communicator */
	long fYcomm;

	/** N-communicator */
	long fNcomm;
};

} /* namespace Tahoe */

#endif /*__PSPASES__ */
#endif /* _PSPASES_MATRIX_T_H_ */
