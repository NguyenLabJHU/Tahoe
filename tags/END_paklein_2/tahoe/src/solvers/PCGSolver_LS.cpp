/* $Id: PCGSolver_LS.cpp,v 1.8 2002-10-20 22:49:47 paklein Exp $ */
/* created: paklein (08/19/1999) */

#include "PCGSolver_LS.h"

#include <iostream.h>
#include <math.h>

#include "fstreamT.h"
#include "toolboxConstants.h"
#include "ExceptionT.h"

#include "FEManagerT.h"
#include "DiagonalMatrixT.h"

/* constructor */

using namespace Tahoe;

PCGSolver_LS::PCGSolver_LS(FEManagerT& fe_manager, int group):
	NLSolver(fe_manager, group),
	fPreconditioner(0) //TEMP
{
	/* check */
	if (fMatrixType != kDiagonalMatrix)
	{
		cout << "\n PCGSolver_LS::PCGSolver_LS: expecting matrix type: "
		     << kDiagonalMatrix << endl;
		throw ExceptionT::kGeneralFail;
	}
	
	/* set assembly mode */
#ifdef __NO_RTTI__
	DiagonalMatrixT* pdiag = (DiagonalMatrixT*) fLHS;
#else
	DiagonalMatrixT* pdiag = dynamic_cast<DiagonalMatrixT*>(fLHS);
	if (!pdiag)
	{
		cout << "\n PCGSolver_LS::PCGSolver_LS: unable to cast LHS matrix to\n"
		     <<   "     DiagonalMatrixT" << endl;
		throw ExceptionT::kGeneralFail;
	}
#endif
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

/* (re-)configure the global equation system */
void PCGSolver_LS::Initialize(int tot_num_eq, int loc_num_eq, int start_eq)
{
		/* inherited */
	NLSolver::Initialize(tot_num_eq, loc_num_eq, start_eq);

	/* allocate work space */
	fdiff_R.Dimension(fRHS.Length());
}

/*************************************************************************
* Protected
*************************************************************************/

double PCGSolver_LS::SolveAndForm(bool newtangent)
{		
#pragma unused(newtangent)

	/* form the stiffness matrix */
	if (fNumIteration == 0 || fPreconditioner)
	{
		fLHS->Clear();
		fFEManager.FormLHS(Group());
		fPreconditioner = 0;
	}

	/* get new search direction (in fRHS) */
	fR = fRHS;
	CGSearch();

	/* apply update to system */
	Update(fRHS, &fR);
								
	/* compute new residual */
	fRHS = 0.0;
	fFEManager.FormRHS(Group());
	
	/* combine residual magnitude with update magnitude */
	/* e = a1 |R| + a2 |delta_d|                        */
	//not implemented!
			
	return Residual(fRHS);
}

/*************************************************************************
* Private
*************************************************************************/

void PCGSolver_LS::CGSearch(void)
{
	/* restart */
	if (fmod(double(fNumIteration), fRestart) < kSmall)
	{
		fR_last = fRHS;
		if (!fLHS->Solve(fRHS)) throw ExceptionT::kBadJacobianDet;
		fu_last = fRHS;
		
		/* reform preconditioner */
		if (fNumIteration > 0) fPreconditioner = 1;

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
		if (!fLHS->Solve(fdiff_R)) throw ExceptionT::kBadJacobianDet; /* apply scaling */
		double beta = InnerProduct(fRHS, fdiff_R);
		fdiff_R = fR_last;     /* copy */
		if (!fLHS->Solve(fdiff_R)) throw ExceptionT::kBadJacobianDet; /* apply scaling */
		beta /= InnerProduct(fR_last, fdiff_R);
		
		/* limit beta */
		//beta = (beta < 0.0) ? 0.0 : beta;
		
		/* compute new update (in last update) */
		fR_last = fRHS;
		if (!fLHS->Solve(fRHS)) throw ExceptionT::kBadJacobianDet; /* apply preconditioner */
		fRHS.AddScaled(beta, fu_last);
		fu_last = fRHS;

		/* output control */
		fVerbose = 0;
	}
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
