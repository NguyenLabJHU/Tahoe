/* $Id: GlobalMatrixT.cpp,v 1.14.22.1 2003-11-04 19:47:29 bsun Exp $ */
/* created: paklein (03/23/1997) */
#include "GlobalMatrixT.h"
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include "toolboxConstants.h"
#include "dArrayT.h"

using namespace Tahoe;

/* cconstructor */
GlobalMatrixT::GlobalMatrixT(ostream& out, int check_code):
	fOut(out),
	fCheckCode(check_code),
	fLocNumEQ(0),	
	fTotNumEQ(0),
	fStartEQ(0),
	fIsFactorized(0)
{

}

GlobalMatrixT::GlobalMatrixT(const GlobalMatrixT& source):
	fOut(source.fOut),
	fCheckCode(kNoCheck),
	fLocNumEQ(0),	
	fTotNumEQ(0),
	fStartEQ(0),
	fIsFactorized(0)
{
	GlobalMatrixT::operator=(source);
}

GlobalMatrixT::~GlobalMatrixT(void) { }

/*
* Set the diagonal position matrix and allocate space.
* for the matrix.
*/
void GlobalMatrixT::Initialize(int tot_num_eq, int loc_num_eq, int start_eq)
{
	/* set dimensions */
	fTotNumEQ = tot_num_eq;
	fLocNumEQ = loc_num_eq;
	fStartEQ  = start_eq;

	/* output */
	fOut << "\n E q u a t i o n    S y s t e m    D a t a :\n\n";
	fOut << " Local number of equations . . . . . . . . . . . = " << fLocNumEQ << '\n';
	fOut << " Total number of equations . . . . . . . . . . . = " << fTotNumEQ << '\n';
	fOut.flush();

	/* consistency */
	const char caller[] = "GlobalMatrixT::Initialize";
	if (fLocNumEQ > fTotNumEQ)
		ExceptionT::GeneralFail(caller, "local number of equations %d cannot exceed the total number of equations %d", fLocNumEQ, fTotNumEQ);

	/* must have at least 1 active equation */
	if (fLocNumEQ < 1)
		cout << "\n GlobalMatrixT::Initialize: WARNING: no active equations" << endl;
 	else if (fStartEQ < 1) /* active equation numbers must be > 0 */
		ExceptionT::GeneralFail(caller, "active equation numbers must be > 0");
}

/* set all matrix values to 0.0 */
void GlobalMatrixT::Clear(void) { fIsFactorized = 0; }

/*
* Solve the system for the vector given, returning the result
* in the same array
*/
bool GlobalMatrixT::Solve(dArrayT& result)
{
	/* catch any exceptions */
	try 
	{
		if (!fIsFactorized)
		{
			/* store original precision */
			int old_precision = fOut.precision();
	
			/* rank checks before factorization */
			fOut.precision(12);
			PrintLHS();
			fOut.precision(old_precision);
	
			/* factorize */
			fIsFactorized = 1;
			Factorize();
	
			/* rank checks after factorization */
			fOut.precision(12);
			PrintZeroPivots();
			PrintAllPivots();
			fOut.precision(old_precision);
		}

		/* output before solution */
		PrintRHS(result);

		/* find new search direction */
		BackSubstitute(result);

		/* output after solution */
		PrintSolution(result);
	}
	catch (ExceptionT::CodeT error) {
		cout << "\n GlobalMatrixT::Solve: caught exception: " << error << endl;
		return false;
	}
	return true;
}

/* strong manipulation functions 
 * NOTE: These must be overridden to provide support for these functions.
 *       By default, these all throw ExceptionT::xceptions. These could be pure
 *       virtual, but that requires updating all derived matrix types */
void GlobalMatrixT::OverWrite(const ElementMatrixT& elMat, const nArrayT<int>& eqnos)
{
#pragma unused(elMat)
#pragma unused(eqnos)
	ExceptionT::GeneralFail("GlobalMatrixT::OverWrite", "not implemented");
}

void GlobalMatrixT::Disassemble(dMatrixT& elMat, const nArrayT<int>& eqnos) const
{
#pragma unused(elMat)
#pragma unused(eqnos)
	ExceptionT::GeneralFail("GlobalMatrixT::Disassemble", "not implemented");
}

void GlobalMatrixT::DisassembleDiagonal(dArrayT& diagonals, const nArrayT<int>& eqnos) const
{
#pragma unused(diagonals)
#pragma unused(eqnos)
	ExceptionT::GeneralFail("GlobalMatrixT::DisassembleDiagonal", "not implemented");
}

/* assignment operator */
GlobalMatrixT& GlobalMatrixT::operator=(const GlobalMatrixT& RHS)
{
	fCheckCode    = RHS.fCheckCode;
	fLocNumEQ     = RHS.fLocNumEQ;
	fTotNumEQ     = RHS.fTotNumEQ;
	fStartEQ      = RHS.fStartEQ;
	fIsFactorized = RHS.fIsFactorized;
	return *this;
}

/* matrix-vector product */
bool GlobalMatrixT::Multx(const dArrayT& x, dArrayT& b) const 
{ 
#pragma unused(x)
#pragma unused(b)
	return false; 
}

/* Tranpose[matrix]-vector product */
bool GlobalMatrixT::MultTx(const dArrayT& x, dArrayT& b) const 
{
#pragma unused(x)
#pragma unused(b)
	return false; 
}

/* return the values along the diagonal of the matrix */
bool GlobalMatrixT::CopyDiagonal(dArrayT& diags) const
{
#pragma unused(diags)
	return false;
}

/**************************************************************************
* Protected
**************************************************************************/

void GlobalMatrixT::PrintRHS(const dArrayT& RHS) const
{
	if (fCheckCode != kPrintRHS) return;
	
	/* increase output stream precision */
	double* p = RHS.Pointer();

	int high_precision = 12;
	fOut.precision(high_precision);
	int d_width = OutputWidth(fOut, p);

	fOut << "\n RHS vector:\n\n";
	fOut << setw(kIntWidth)    << "loc eq."
	     << setw(kIntWidth)    << "glb eq." 
	     << setw(d_width)      << "RHS\n\n";
	for (int i = 0; i < fLocNumEQ; i++)
		fOut << setw(kIntWidth) << i + 1
		     << setw(kIntWidth) << fStartEQ + i
		     << setw(d_width) << *p++ << '\n';	
	fOut << endl;

	/* restore stream precision */
	fOut.precision(kPrecision);
}

void GlobalMatrixT::PrintSolution(const dArrayT& solution) const
{
	if (fCheckCode != kPrintSolution) return;
	
	/* increase output stream precision */
	double* p = solution.Pointer();

	int high_precision = 12;
	fOut.precision(high_precision);
	int d_width = OutputWidth(fOut, p);

	fOut << "\n solution vector:\n\n";
	fOut << setw(kIntWidth)    << "loc eq."
	     << setw(kIntWidth)    << "glb eq." 
	     << setw(d_width)      << "solution\n\n";
	for (int i = 0; i < fLocNumEQ; i++)
		fOut << setw(kIntWidth) << i + 1
		     << setw(kIntWidth) << fStartEQ + i
		     << setw(d_width) << *p++ << '\n';	
	fOut << endl;

	/* restore stream precision */
	fOut.precision(kPrecision);
}
