/* $Id: SolverT.cpp,v 1.11.2.1 2002-12-27 23:09:10 paklein Exp $ */
/* created: paklein (05/23/1996) */
#include "SolverT.h"

#include <iostream.h>
#include <string.h>

#include "fstreamT.h"
#include "FEManagerT.h"
#include "CommunicatorT.h"
#include "iArrayT.h"

/* global matrix */
#include "CCSMatrixT.h"
#include "DiagonalMatrixT.h"
#include "FullMatrixT.h"
#include "CCNSMatrixT.h"
#include "AztecMatrixT.h"
#include "SLUMatrix.h"
#include "SPOOLESMatrixT.h"

#ifdef __TAHOE_MPI__
#include "SPOOLESMatrixT_mpi.h"
#endif

using namespace Tahoe;

/* constructor */
SolverT::SolverT(FEManagerT& fe_manager, int group):
	fFEManager(fe_manager),
	fGroup(group),
	fLHS(NULL),
	fNumIteration(0)
{
	/* read parameters */
	ifstreamT& in = fFEManager.Input();
	int check_code;
	in >> fMatrixType;
	in >> fPrintEquationNumbers;
	in >> check_code;

	ostream& out = fFEManager.Output();
	out << "\n S o l v e r   p a r a m e t e r s:\n\n";
	out << " Group . . . . . . . . . . . . . . . . . . . . . = " << fGroup << '\n';
	out << " Global equation type. . . . . . . . . . . . . . = " << fMatrixType << '\n';
	out << "    eq. " << kDiagonalMatrix   << ", diagonal matrix\n";
	out << "    eq. " << kProfileSolver    << ", profile solver (symmetric and nonsymmetric)\n";
	out << "    eq. " << kFullMatrix       << ", full matrix (most general)\n";   	

#ifdef __AZTEC__
	out << "    eq. " << kAztec            << ", Aztec-based, sparse matrix with iterative solvers\n";   	
#else
	out << "    eq. " << kAztec            << ", NOT AVAILABLE\n";
#endif

#ifdef __SUPERLU__
	out << "    eq. " << kSparseDirect     << ", fully sparse matrix with direct solver: SuperLU\n";
#else
	out << "    eq. " << kSparseDirect     << ", NOT AVAILABLE\n";
#endif

#ifdef __SPOOLES__
	out << "    eq. " << kSPOOLES     << ", sparse matrix with direct solver: SPOOLES\n";
#else
	out << "    eq. " << kSPOOLES     << ", NOT AVAILABLE\n";
#endif

	out << " Output global equation numbers. . . . . . . . . = " << fPrintEquationNumbers << '\n';
	out << " Check code. . . . . . . . . . . . . . . . . . . = " << check_code << '\n';
	out << "    eq. " << GlobalMatrixT::kNoCheck       << ", do not perform rank check\n";
	out << "    eq. " << GlobalMatrixT::kZeroPivots    << ", zero/negative pivots\n";
	out << "    eq. " << GlobalMatrixT::kAllPivots     << ", all pivots\n";
	out << "    eq. " << GlobalMatrixT::kPrintLHS      << ", entire LHS matrix\n";
	out << "    eq. " << GlobalMatrixT::kPrintRHS      << ", entire RHS vector\n";
	out << "    eq. " << GlobalMatrixT::kPrintSolution << ", solution vector\n";   	

	/* check matrix type against analysis code */
	if (fPrintEquationNumbers != 0 && fPrintEquationNumbers != 1)
	{	
		cout << "\n SolverT::SolverT: \"print equation numbers\" out of range: {0,1}" << endl;
		throw ExceptionT::kBadInputValue;
	}
//	if (!CheckMatrixType(fMatrixType, fFEManager.Analysis()))
//	{
//		cout << "\n SolverT::SolverT: incompatible matrix type: " << fMatrixType << endl;		
//		throw ExceptionT::kBadInputValue;
//	}
	
	/* construct global matrix */
	SetGlobalMatrix(fMatrixType, check_code);
	
	/* console */
	iSetName("solver");
	iAddVariable("print_equation_numbers", fPrintEquationNumbers);
}

/* destructor */
SolverT::~SolverT(void) { delete fLHS; }

/* configure the global equation system */
void SolverT::Initialize(int tot_num_eq, int loc_num_eq, int start_eq)
{	
	try {
		/* allocate rhs vector */
		fRHS.Dimension(loc_num_eq);
		fRHS = 0.0;
		
		/* set global equation matrix type */
		fLHS->Initialize(tot_num_eq, loc_num_eq, start_eq);
	
		/* output global equation number for each DOF */
		if (fPrintEquationNumbers) fFEManager.WriteEquationNumbers(fGroup);
	}	

	catch (ExceptionT::CodeT error_code)
	{
		cout << "\n SolverT::Initialize: exception: "
		     << ExceptionT::ToString(error_code) << endl;
		throw error_code;
	}
}

/* error handler */
void SolverT::ResetStep(void) { /* do nothing */ }

/* process element group equation data to configure GlobalMatrixT */
void SolverT::ReceiveEqns(const iArray2DT& equations) const
{
	fLHS->AddEquationSet(equations);
}

void SolverT::ReceiveEqns(const RaggedArray2DT<int>& equations) const
{
	fLHS->AddEquationSet(equations);
}

void SolverT::AssembleRHS(const nArrayT<double>& elRes, const nArrayT<int>& eqnos)
{
	/* consistency check */
	if (elRes.Length() != eqnos.Length()) throw ExceptionT::kGeneralFail;

#if __option(extended_errorcheck)
	GlobalT::EquationNumberScopeT scope = (GlobalT::EquationNumberScopeT) fLHS->EquationNumberScope();
#endif

	int num_eq = fLHS->NumEquations();
	int start_eq = fLHS->StartEquation();
	for (int i = 0; i < eqnos.Length(); i++)
	{
		int eq = eqnos[i] - start_eq;
	
		/* in range */
		if (eq > -1 && eq < num_eq) fRHS[eq] += elRes[i];

#if __option(extended_errorcheck)
		else if (scope == GlobalT::kLocal && eq >= num_eq)
		{
			cout << "\n SolverT::AssembleRHS: equation number is out of range: "
			     << eq + start_eq << endl;
			throw ExceptionT::kOutOfRange;
		}
#endif
	}	
}

void SolverT::OverWriteRHS(const dArrayT& elRes, const nArrayT<int>& eqnos)
{
	/* consistency check */
	if (elRes.Length() != eqnos.Length()) throw ExceptionT::kGeneralFail;

	int num_eq = fLHS->NumEquations();
	int start_eq = fLHS->StartEquation();
	for (int i = 0; i < eqnos.Length(); i++)
	{
		int eq = eqnos[i] - start_eq;
	
		/* in range */
		if (eq > -1 && eq < num_eq) fRHS[eq] = elRes[i];
	}	
}

void SolverT::DisassembleRHS(dArrayT& elRes, const nArrayT<int>& eqnos) const
{
	/* consistency check */
	if (elRes.Length() != eqnos.Length()) throw ExceptionT::kGeneralFail;

	int num_eq = fLHS->NumEquations();
	int start_eq = fLHS->StartEquation();
	for (int i = 0; i < eqnos.Length(); i++)
	{
		int eq = eqnos[i] - start_eq;
	
		/* in range */
		if (eq > 0 && eq < num_eq)
			elRes[i] = fRHS[eq];
		else
			elRes[i] = 0.0;
	}	
}

/* return the required equation numbering scope - local by default */
GlobalT::EquationNumberScopeT SolverT::EquationNumberScope(void) const
{
#if __option(extended_errorcheck)
	if (!fLHS)
	{
		cout << "\n SolverT::EquationNumberScope: invalid LHS" << endl;
		throw ExceptionT::kGeneralFail;	
	}
#endif

	return (GlobalT::EquationNumberScopeT) fLHS->EquationNumberScope();
}

/*************************************************************************
* Protected
*************************************************************************/

/* advance to next load step. Returns 0 if there are no more
* steps. Overload to add class dependent initializations */
//int SolverT::Step(void) { return fFEManager.Step(); }

/* return the magnitude of the residual force */
double SolverT::Residual(const dArrayT& force) const
{
	return sqrt(InnerProduct(force,force));
}

/* (distributed) inner product */	
double SolverT::InnerProduct(const dArrayT& v1, const dArrayT& v2) const
{
	/* check heart beat */
	if (fFEManager.Communicator().Sum(ExceptionT::kNoError) != 0) throw ExceptionT::kBadHeartBeat;

	return fFEManager.Communicator().Sum(dArrayT::Dot(v1, v2));
}

/*************************************************************************
* Private
*************************************************************************/

/* check matrix type against analysis code, return 1 if
* compatible, 0 otherwise */
int SolverT::CheckMatrixType(int matrix_type, int analysis_code) const
{
	int OK = 1;
	switch (matrix_type)
	{
		case kDiagonalMatrix:
		
			OK = (analysis_code == GlobalT::kLinExpDynamic   ||
			      analysis_code == GlobalT::kNLExpDynamic    ||
			      analysis_code == GlobalT::kVarNodeNLExpDyn ||
			      analysis_code == GlobalT::kNLExpDynKfield);
			break;
		
		case kProfileSolver:
		
			OK = (analysis_code == GlobalT::kLinStatic       ||
			      analysis_code == GlobalT::kLinDynamic      ||
			      analysis_code == GlobalT::kNLStatic        ||
			      analysis_code == GlobalT::kNLDynamic       ||
			      analysis_code == GlobalT::kDR              ||
			      analysis_code == GlobalT::kNLStaticKfield  ||
			      analysis_code == GlobalT::kVarNodeNLStatic ||
			      analysis_code == GlobalT::kAugLagStatic);
			break;

		case kFullMatrix:

			/* not for explicit dynamics */
			OK = (analysis_code != GlobalT::kLinExpDynamic &&
			      analysis_code != GlobalT::kNLExpDynamic  &&
			      analysis_code != GlobalT::kVarNodeNLExpDyn);
			break;
					
		case kAztec:

			/* not for explicit dynamics */
			OK = (analysis_code != GlobalT::kLinExpDynamic &&
			      analysis_code != GlobalT::kNLExpDynamic  &&
			      analysis_code != GlobalT::kVarNodeNLExpDyn);
			break;
			
		case kSparseDirect:
		
			OK = (analysis_code == GlobalT::kLinStatic       ||
			      analysis_code == GlobalT::kLinDynamic      ||
			      analysis_code == GlobalT::kNLStatic        ||
			      analysis_code == GlobalT::kNLDynamic       ||
			      analysis_code == GlobalT::kDR              ||
			      analysis_code == GlobalT::kNLStaticKfield  ||
			      analysis_code == GlobalT::kVarNodeNLStatic ||
			      analysis_code == GlobalT::kAugLagStatic);
			break;

		case kSPOOLES:
		
			OK = (analysis_code == GlobalT::kLinStatic       ||
			      analysis_code == GlobalT::kLinDynamic      ||
			      analysis_code == GlobalT::kNLStatic        ||
			      analysis_code == GlobalT::kNLDynamic       ||
			      analysis_code == GlobalT::kDR              ||
			      analysis_code == GlobalT::kNLStaticKfield  ||
			      analysis_code == GlobalT::kVarNodeNLStatic ||
			      analysis_code == GlobalT::kAugLagStatic);		
			break;

		default:

			cout << "\n SolverT::CheckMatrixType: unknown matrix type ";
			cout << matrix_type << '\n';
			OK = 0;		
	}

	/* compatibility */
	if (!OK)
	{
		cout << "\n SolverT::CheckMatrixType: matrix type " << matrix_type << '\n';
		cout << " is not compatible with analysis code " << analysis_code << endl;

		ostream& out = fFEManager.Output();
		out << "\n SolverT::CheckMatrixType: matrix type " << matrix_type << '\n';
		out << " is not compatible with analysis code " << analysis_code << endl;
	}
	else
		return 1;

	/* checks */
	if (fFEManager.Size() > 1 &&
	    (matrix_type == kFullMatrix    ||
	     matrix_type == kProfileSolver ||
	     matrix_type == kSparseDirect  ||
	     matrix_type == kSPOOLES))
	{
		cout << "\n SolverT::CheckMatrixType: matrix type not support in parallel: "
		     << matrix_type << endl;
		throw ExceptionT::kGeneralFail;
	}
	return OK;
}

/* set global equation matrix */
void SolverT::SetGlobalMatrix(int matrix_type, int check_code)
{
	/* streams */
	ifstreamT& in = fFEManager.Input();
	ostream&   out = fFEManager.Output();
	switch (matrix_type)
	{
		case kDiagonalMatrix:

			fLHS = new DiagonalMatrixT(out, check_code, DiagonalMatrixT::kNoAssembly);
			break;

		case kProfileSolver:
		{
			/* global system properties */
			GlobalT::SystemTypeT type = fFEManager.GlobalSystemType(fGroup);
		
			if (type == GlobalT::kNonSymmetric)
				fLHS = new CCNSMatrixT(out, check_code);
			else if (type == GlobalT::kSymmetric)
				fLHS = new CCSMatrixT(out, check_code);
			else
			{
				cout << "\n SolverT::SetGlobalMatrix: global system type " << type;
				cout << " is not\n";
				cout <<   "     compatible with matrix type " << kProfileSolver;
				cout << endl;
			}
			break;
		}
		case kFullMatrix:
		
			fLHS = new FullMatrixT(out, check_code);
			break;

		case kAztec:
		{
#ifdef __AZTEC__
			/* construct */
			fLHS = new AztecMatrixT(in, out, check_code);
#else
			cout << "\n SolverT::SetGlobalMatrix: Aztec solver not installed: ";
			cout << fMatrixType << endl;
			throw ExceptionT::kGeneralFail;		
#endif /* __AZTEC__ */
			break;
		}

		case kSparseDirect:
		{
#ifdef __SUPERLU__
			// when spd code is in place, check matrix type as
			// above in kProfileSolver. For now, always go with
			// SuperLU.
			fLHS = new SLUMatrix(out, check_code);
#else
			cout << "\n SolverT::SetGlobalMatrix: SuperLU matrix not installed: ";
			cout << fMatrixType << endl;
			throw ExceptionT::kGeneralFail;
#endif /* __SUPERLU__ */
			break;
		}

		case kSPOOLES:
		{
#ifdef __SPOOLES__
			/* global system properties */
			GlobalT::SystemTypeT type = fFEManager.GlobalSystemType(fGroup);

			/* solver options */
			bool pivoting = true; //NOTE: SPOOLES v2.2 does not seem to solve non-symmetric
			                      //      systems correctly in parallel if pivoting is disabled
			bool symmetric;
			if (type == GlobalT::kDiagonal || type == GlobalT::kSymmetric)
				symmetric = true;
			else if (type == GlobalT::kNonSymmetric)
				symmetric = false;
			else
			{
				cout << "\n SolverT::SetGlobalMatrix: unexpected system type: "
				     << type << endl;
				throw ExceptionT::kGeneralFail;
			}

#ifdef __TAHOE_MPI__
#ifdef __MWERKS__

			cout << "\n SolverT::SetGlobalMatrix: SPOOLES requires functions not supported\n"
			     <<   "     in MacMPI" << endl;
			throw ExceptionT::kBadInputValue;
#else
			/* constuctor */
			if (fFEManager.Size() > 1)
			{
#ifdef __SPOOLES_MPI__
				fLHS = new SPOOLESMatrixT_mpi(out, check_code, symmetric, pivoting, fFEManager.Communicator());
#else
				cout << "\n SolverT::SetGlobalMatrix: SPOOLES MPI not installed: ";
				cout << matrix_type << endl;
				throw ExceptionT::kGeneralFail;
#endif /* __SPOOLES_MPI__ */
			}
			else
				fLHS = new SPOOLESMatrixT(out, check_code, symmetric, pivoting);
#endif /* __MWERKS__ */
#else
			/* constuctor */
			fLHS = new SPOOLESMatrixT(out, check_code, symmetric, pivoting);

#endif /* __TAHOE_MPI__ */
#else
			cout << "\n SolverT::SetGlobalMatrix: SPOOLES not installed: ";
			cout << matrix_type << endl;
			throw ExceptionT::kGeneralFail;
#endif /* __SPOOLES__ */
			break;
		}
		default:
		
			cout << "\n SolverT::SetGlobalMatrix: unknown matrix type: ";
			cout << matrix_type << endl;
			throw ExceptionT::kGeneralFail;
	}	
	if (!fLHS) throw ExceptionT::kOutOfMemory;
}

/* call for equation renumbering */
bool SolverT::RenumberEquations(void)
{
	if (!fLHS) throw ExceptionT::kGeneralFail;
	return fLHS->RenumberEquations();
}
