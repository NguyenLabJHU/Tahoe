/* $Id: PCGSolver_LS.cpp,v 1.19.2.2 2004-03-24 01:58:35 paklein Exp $ */
/* created: paklein (08/19/1999) */
#include "PCGSolver_LS.h"

#include <iostream.h>
#include <math.h>

#include "fstreamT.h"
#include "toolboxConstants.h"
#include "ExceptionT.h"

#include "FEManagerT.h"
#include "DiagonalMatrixT.h"

using namespace Tahoe;

/* constructor */
PCGSolver_LS::PCGSolver_LS(FEManagerT& fe_manager, int group):
	NLSolver(fe_manager, group),
	fRestart_count(-1)
{
	SetName("PCG_solver");

	/* set console */
	iAddVariable("search_iterations", fSearchIterations);
	iAddVariable("line_search_tolerance", fOrthogTolerance);
	iAddVariable("max_step_size", fMaxStepSize);
	iAddVariable("restart_count", fRestart);	
}

#if 0
PCGSolver_LS::PCGSolver_LS(FEManagerT& fe_manager, int group):
	NLSolver(fe_manager, group),
	fRestart_count(-1)
//	,fPreconditioner(0) //TEMP
{
	SetName("PCG_solver");

	/* check */
	if (fMatrixType != kDiagonalMatrix)
	{
		cout << "\n PCGSolver_LS::PCGSolver_LS: expecting matrix type: "
		     << kDiagonalMatrix << endl;
		throw ExceptionT::kGeneralFail;
	}
	
	/* set assembly mode */
	DiagonalMatrixT* pdiag = TB_DYNAMIC_CAST(DiagonalMatrixT*, fLHS);
	if (!pdiag)
	{
		cout << "\n PCGSolver_LS::PCGSolver_LS: unable to cast LHS matrix to\n"
		     <<   "     DiagonalMatrixT" << endl;
		throw ExceptionT::kGeneralFail;
	}
	pdiag->SetAssemblyMode(DiagonalMatrixT::kDiagOnly);

	ifstreamT& in = fFEManager.Input();
	/* read parameters */
	in >> fRestart;
	in >> fSearchIterations;
	in >> fOrthogTolerance;
	in >> fMaxStepSize;

	/* mininum search iterations > 0 */
	fSearchIterations = (fSearchIterations != 0 &&
	                     fSearchIterations < 3) ? 3 : fSearchIterations;

	/* print parameters */
	ostream& out = fFEManager.Output();
	out << " CG restart count. . . . . . . . . . . . . . . . = " << fRestart          << '\n';
	out << " Maximum number of line search iterations. . . . = " << fSearchIterations << '\n';
	out << " Line search orthoginality tolerance . . . . . . = " << fOrthogTolerance  << '\n';
	out << " Maximum update step size. . . . . . . . . . . . = " << fMaxStepSize      << endl;
	
	/* checks */
	if (fRestart < 1)           throw ExceptionT::kBadInputValue;
	if (fSearchIterations < 0)  throw ExceptionT::kBadInputValue;
	if (fOrthogTolerance > 1.0) throw ExceptionT::kBadInputValue;
	if (fMaxStepSize      < 0)  throw ExceptionT::kBadInputValue;
	
	/* allocate space for history */
	fSearchData.Dimension(fSearchIterations, 2);

	/* set console */
	iAddVariable("search_iterations", fSearchIterations);
	iAddVariable("line_search_tolerance", fOrthogTolerance);
	iAddVariable("max_step_size", fMaxStepSize);
	iAddVariable("restart_count", fRestart);
}
#endif

/* (re-)configure the global equation system */
void PCGSolver_LS::Initialize(int tot_num_eq, int loc_num_eq, int start_eq)
{
	/* inherited */
	NLSolver::Initialize(tot_num_eq, loc_num_eq, start_eq);

	/* allocate work space */
	fdiff_R.Dimension(fRHS.Length());
	
	/* set flag */
	fRestart_count = -1;
}

/* describe the parameters needed by the interface */
void PCGSolver_LS::DefineParameters(ParameterListT& list) const
{
	/* inherited */
	NLSolver::DefineParameters(list);

	/* set default matrix type to diagonal */
	ParameterT& matrix_type = list.GetParameter("matrix_type");
	matrix_type.SetDefault(kDiagonalMatrix);

	/* restart iterations */
	ParameterT restart(ParameterT::Integer, "restart");
	restart.AddLimit(0, LimitT::LowerInclusive);
	list.AddParameter(restart);

	/* line search iterations */
	ParameterT line_search_iterations(ParameterT::Integer, "line_search_iterations");
	line_search_iterations.SetDefault(3);
	list.AddParameter(line_search_iterations);

	/* line search orthogonality tolerance */
	ParameterT line_search_tolerance(ParameterT::Double, "line_search_tolerance");
	line_search_tolerance.SetDefault(0.25);
	list.AddParameter(line_search_tolerance);

	/* maximum step size */
	ParameterT max_step(ParameterT::Double, "max_step");
	max_step.SetDefault(2.5);
	list.AddParameter(max_step);
}

/* accept parameter list */
void PCGSolver_LS::TakeParameterList(const ParameterListT& list)
{
	/* inherited */
	NLSolver::TakeParameterList(list);

	/* extract parameters */
	fRestart = list.GetParameter("restart");
	fSearchIterations = list.GetParameter("line_search_iterations");
	fOrthogTolerance = list.GetParameter("line_search_tolerance");
	fMaxStepSize = list.GetParameter("max_step");

	/* allocate space for history */
	fSearchData.Dimension(fSearchIterations, 2);
}

/*************************************************************************
 * Protected
 *************************************************************************/

double PCGSolver_LS::SolveAndForm(int& iteration)
{
	/* form the stiffness matrix (must be cleared previously) */
	if (fLHS_update) {
		fLHS_lock = kOpen;
		fFEManager.FormLHS(Group(), fLHS->MatrixType());
		fLHS_lock = kLocked;
	}

	/* get new search direction (in fRHS) */
	fR = fRHS;
	CGSearch();

	/* apply update to system */
	fRHS_lock = kOpen;
	fLHS_lock = kIgnore;
	Update(fRHS, &fR);
	fLHS_lock = kLocked;
									
	/* recalculate residual */
	iteration++;
	if (fLHS_update) {
		fLHS->Clear();
		fLHS_lock = kOpen; /* LHS open for assembly, too! */
	}
	else
		fLHS_lock = kIgnore; /* ignore assembled values */
	fRHS = 0.0;
	fFEManager.FormRHS(Group());	
	fLHS_lock = kLocked;
	fRHS_lock = kLocked;
	
	/* could combine residual magnitude with update magnitude
	 * e = a1 |R| + a2 |delta_d|  --> not implemented */	
	return Residual(fRHS);
}

SolverT::SolutionStatusT PCGSolver_LS::Solve(int max_iterations)
{
	fRestart_count = -1;
	fVerbose = 0;

	/* inherited */
	return NLSolver::Solve(max_iterations);
}

/*************************************************************************
 * Private
 *************************************************************************/

void PCGSolver_LS::CGSearch(void)
{
	const char caller[] = "PCGSolver_LS::CGSearch";

	/* restart */
	fRestart_count++;
	if (fRestart_count == 0 || fRestart_count == fRestart) {
		fR_last = fRHS;
		if (!fLHS->Solve(fRHS)) ExceptionT::BadJacobianDet(caller);
		fu_last = fRHS;
		fRestart_count = 0;

		/* output control */
		fVerbose = 1;
	}
	else
	{
		fdiff_R.DiffOf(fRHS, fR_last);

		/* Gill & Murray (4.88) */
//		double beta =-InnerProduct(fRHS, fdiff_R)/
//                    InnerProduct(fdiff_R, fu_last);			
			 			              			
		/* Bertsekas (6.36) and Polak-Ribiere formula (JAS3D) */
//		double beta = InnerProduct(fRHS, fdiff_R)/
//		              InnerProduct(fR_last, fR_last);

		/* with scaling matrix Bertsekas (6.32) */
		if (!fLHS->Solve(fdiff_R)) ExceptionT::BadJacobianDet(caller); /* apply scaling */
		double beta = InnerProduct(fRHS, fdiff_R);
		fdiff_R = fR_last;     /* copy */
		if (!fLHS->Solve(fdiff_R)) ExceptionT::BadJacobianDet(caller); /* apply scaling */

		/* no division by zero */
		double denominator = InnerProduct(fR_last, fdiff_R);
		if (fabs(denominator) < 1.0e-24) {
			denominator = 1.0;
			beta = 0.0; /* revert to steepest descent */
		}
		else
			beta /= denominator;		
		
		/* limit beta */
		//beta = (beta < 0.0) ? 0.0 : beta;
		
		/* compute new update (in last update) */
		fR_last = fRHS;
		if (!fLHS->Solve(fRHS)) ExceptionT::BadJacobianDet(caller); /* apply preconditioner */
		fRHS.AddScaled(beta, fu_last);
		fu_last = fRHS;

		/* output control */
		fVerbose = 0;
	}

	/* check recalculation of LHS */
	if (fRestart_count == (fRestart - 1))
		fLHS_update = true;
	else
		fLHS_update = false;
}

void PCGSolver_LS::Update(const dArrayT& update, const dArrayT* residual)
{
	/* inherited (no line search) */
	if (fSearchIterations == 0)
	{
		NLSolver::Update(update, residual);
		return;
	}

	/* initialize */
	fUpdate     = update;
	s_current   = 0.0;
	fSearchData = 0.0;

	/* first secant point */
	double s_a;
	double G_a;
	if (residual)
	{
		s_a = 0.0;
		G_a = InnerProduct(fUpdate, *residual);
	}
	else
	{
		s_a = 0.5;
		G_a = GValue(s_a);
	}	
	fSearchData(0,0) = s_a;
	fSearchData(0,1) = G_a;

	/* check full step */
	double s_b = 1.0;
	double G_b = GValue(s_b);
	fSearchData(1,0) = s_b;
	fSearchData(1,1) = G_b;
	
	/* minimize G with secant method */		
	double G_0 = (fabs(G_a) > fabs(G_b)) ? G_b : G_a;
	int count = 2;
	double G_new;
	bool give_up = false;
	do {
		double m = (G_a - G_b)/(s_a - s_b);
		double b = G_b - m*s_b;
		double s_new = -b/m;
		
		/* exceed step size bounds */
		if (s_new > fMaxStepSize || s_new < 0.0)
		{
			give_up = true;
			
			/* store max step */
			if (s_new > fMaxStepSize)
			{
				s_new = fMaxStepSize;
				G_new = GValue(s_new);
				fSearchData(count, 0) = s_new;
				fSearchData(count, 1) = G_new;
				count++;
			}
			break;
		}
				
		/* update and compute test value */				
		G_new = GValue(s_new);
		fSearchData(count, 0) = s_new;
		fSearchData(count, 1) = G_new;
		if (fabs(G_a) > fabs(G_new) && fabs(G_a) > fabs(G_b))
		{
			G_a = G_new;
			s_a = s_new;
			give_up = false;
		}
		else if (fabs(G_b) > fabs(G_new) && fabs(G_b) > fabs(G_a))
		{
			G_b = G_new;
			s_b = s_new;
			give_up = false;
		}
		else
		{
			/* G_a and G_b don't bracket zero */
			if (G_b*G_a > 0)
			{
				if (G_a*G_new < 0)
				{
					G_a = G_new;
					s_a = s_new;
				}
				else if (G_b*G_new < 0)
				{
					G_b = G_new;
					s_b = s_new;
				}
				else /* exit */
					give_up = true;
			}
			else /* exit */
				give_up = true;
		
		}
		
		/* max iterations */
		if (++count >= fSearchIterations) give_up = true;
		
	} while (fabs(G_new) > fZeroTolerance &&
	         fabs(G_new/G_0) > fOrthogTolerance &&
!give_up);

	/* best step on fail */
	if (give_up)
	{
		/* find "best" step */
		double s_best = fabs(fSearchData(0,0));
		double G_best = fabs(fSearchData(0,1));
		int    best = 0;
		for (int i = 1; i < count; i++)
		{
			double s_test = fabs(fSearchData(i,0));
			double G_test = fabs(fSearchData(i,1));
			if (fabs(s_best) < kSmall || // what is this catching?
			    (s_test > kSmall && G_test < G_best))
			{
				s_best = s_test;
				G_best = G_test;
				best = i;
			}
		}
	
		/* set to "best" */
		GValue(fSearchData(best,0));
	}

	if (fVerbose)
		cout << " LS: " << count << setw(kDoubleWidth) << s_current << " | ";
} 	

/* return the line search weight function for the given step size */
double PCGSolver_LS::GValue(double step)
{
	/* scale update vector */
	fRHS = fUpdate;

	/* scale accounting for current step update */
	fRHS *= (step - s_current);
	s_current = step;
	
	/* compute residual */
	fFEManager.Update(Group(), fRHS);
	fRHS = 0.0;
	try { fFEManager.FormRHS(Group()); }
	catch (ExceptionT::CodeT error)
	{
		cout << "\n PCGSolver_LS::GValue: caught exception" << endl;
		throw error;
	}

	return InnerProduct(fUpdate, fRHS);
}
