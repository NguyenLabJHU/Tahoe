/* $Id: SolverT.h,v 1.8.8.1 2002-11-13 08:31:09 paklein Exp $ */
/* created: paklein (05/23/1996) */
#ifndef _SOLVER_H_
#define _SOLVER_H_

/* environment */
#include "Environment.h"

/* base class */
#include "iConsoleObjectT.h"

/* direct members */
#include "dArrayT.h"
#include "GlobalMatrixT.h"
#include "GlobalT.h"

namespace Tahoe {

/* forward declarations */
class FEManagerT;
class iArrayT;
class iArray2DT;
class dMatrixT;
class ElementMatrixT;
template <class TYPE> class RaggedArray2DT;

/** solver base class. This class is responsible for driving the solution
 * procedure over a given series of time increments. The class controls
 * how the solution is determined over a given time step, what determines
 * if the solution has been found, and how to handle failures to find a
 * solution, or other irregularities that occur during the solution procedure.
 * Derived types instantiate different nonlinear solution procedures. */
class SolverT: public iConsoleObjectT
{
public:

	/** nonlinear solver codes */
	enum SolverTypeT {kNewtonSolver = 0, /**< standard Newton solver */
                   kK0_NewtonSolver = 1, /**< initial tangent, Newton solver */
                   kModNewtonSolver = 2, /**< modified Newton solver (development) */
                    kExpCD_DRSolver = 3, /**< central difference, dynamic relaxation */
                   kNewtonSolver_LS = 4, /**< Newton solver with line search */
                      kPCGSolver_LS = 5, /**< preconditioned, nonlinear conjugate gradient */
                  kiNewtonSolver_LS = 6, /**< interactive Newton solver (with line search) */
                               kNOX = 7, /**< NOX library solver */
                            kLinear = 8, /**< linear problems */
                                kDR = 9  /**< dynamic relaxation */                               
                               };

	/** global matrix types */
	enum MatrixTypeT {kDiagonalMatrix = 0, /**< diagonal matrix for "matrix-free" methods */
	                   kProfileSolver = 1, /**< symmetric and nonsymmetric profile solvers */
	                      kFullMatrix = 2, /**< full matrix with pivoting */
					           kAztec = 3, /**< sparse, iterative solver */
			            kSparseDirect = 4, /**< sparse, direct solver: SuperLU */
			                 kSPOOLES = 5  /**< sparse, direct solver: symbolic factorization */
			                 };

	/** solution status */
	enum SolutionStatusT {kContinue = 0, /**< solution not found after the most recent iteration */
                          kConverged = 1, /**< solution found */
                             kFailed = 2  /**< solution procedure has failed */
                             };

	/** constructor */
	SolverT(FEManagerT& fe_manager, int group);

	/** destructor */
	virtual ~SolverT(void);

	/** (re-)configure the global equation system */
	virtual void Initialize(int tot_num_eq, int loc_num_eq, int start_eq);

	/* process element group equation data to configure matrix */
	void ReceiveEqns(const iArray2DT& equations) const;
	void ReceiveEqns(const RaggedArray2DT<int>& equations) const;

	/** solve the system over the current time increment.
	 * \param num_iterations maximum number of iterations to execute. Hitting this limit
	 *        does not signal a SolverT::kFailed status, unless solver's internal parameters
	 *        also indicate the solution procedure has failed.
	 * \return one of SolverT::IterationsStatusT */
	virtual SolutionStatusT Solve(int num_iterations) = 0;

	/* error handler */
	virtual void ResetStep(void);
	
	/** \name assembling the global equation system */
	/*@{*/
	void AssembleLHS(const ElementMatrixT& elMat, const nArrayT<int>& eqnos);
	void AssembleLHS(const ElementMatrixT& elMat, const nArrayT<int>& row_eqnos,
		const nArrayT<int>& col_eqnos);
	void OverWriteLHS(const ElementMatrixT& elMat, const nArrayT<int>& eqnos);
	void DisassembleLHS(dMatrixT& matrix, const nArrayT<int>& eqnos) const;
	void DisassembleLHSDiagonal(dArrayT& diagonals, const nArrayT<int>& eqnos) const;

	void AssembleRHS(const dArrayT& elRes, const nArrayT<int>& eqnos);
	void OverWriteRHS(const dArrayT& elRes, const nArrayT<int>& eqnos);
	void DisassembleRHS(dArrayT& elRes, const nArrayT<int>& eqnos) const;
	/*@}*/

	/* accessor */
	const int& IterationNumber(void) const;

	/** debugging */
	int Check(void) const;
	const dArrayT& RHS(void) const;

	/* return the required equation numbering scope - local by default */
	GlobalT::EquationNumberScopeT EquationNumberScope(void) const;

	/** returns true if solver prefers reordered equations */
	bool RenumberEquations(void);
	
	/** my group */
	int Group(void) const { return fGroup; };

protected:

	/** return the magnitude of the residual force */
	double Residual(const dArrayT& force) const;

	/** inner product */	
	double InnerProduct(const dArrayT& v1, const dArrayT& v2) const;

private:

	/* check matrix type against analysis code, return 1 if
	 * compatible, 0 otherwise */
	int CheckMatrixType(int matrix_type, int analysis_code) const;

	/* set global equation matrix */
	void SetGlobalMatrix(int matrix_type, int check_code);
		 	
protected:

	/** the Boss */	
	FEManagerT& fFEManager;

	/** equation group number */
	int fGroup;

	/* flags */
	int fMatrixType;
	int fPrintEquationNumbers;
	
	/** global equation system */
	/*@{*/
	GlobalMatrixT* fLHS;	
	dArrayT        fRHS;
	/*@}*/

	/** runtime data */
	int fNumIteration;
};

/* inlines */

/* assembling the global equation system */
inline void SolverT::AssembleLHS(const ElementMatrixT& elMat, const nArrayT<int>& eqnos)
{
	fLHS->Assemble(elMat, eqnos);
}

inline void SolverT::AssembleLHS(const ElementMatrixT& elMat, const nArrayT<int>& row_eqnos,
	const nArrayT<int>& col_eqnos)
{
	fLHS->Assemble(elMat, row_eqnos, col_eqnos);
}

inline void SolverT::OverWriteLHS(const ElementMatrixT& elMat, const nArrayT<int>& eqnos)
{
	fLHS->OverWrite(elMat, eqnos);
}

inline void SolverT::DisassembleLHS(dMatrixT& matrix, const nArrayT<int>& eqnos) const
{
	fLHS->Disassemble(matrix, eqnos);
}

inline void SolverT::DisassembleLHSDiagonal(dArrayT& diagonals, const nArrayT<int>& eqnos) const
{
	fLHS->DisassembleDiagonal(diagonals, eqnos);
}

/* debugging */
inline int SolverT::Check(void) const { return fLHS->CheckCode(); }
inline const dArrayT& SolverT::RHS(void) const { return fRHS; }

/* accessor */
inline const int& SolverT::IterationNumber(void) const { return fNumIteration; }

} // namespace Tahoe 
#endif /* _SOLVER_H_ */
