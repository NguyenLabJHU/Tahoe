/* $Id: NLSolver_LS.cpp,v 1.5.4.2 2002-10-20 18:07:51 paklein Exp $ */
/* created: paklein (08/18/1999) */

#include "NLSolver_LS.h"

#include <iostream.h>
#include <math.h>

#include "fstreamT.h"
#include "toolboxConstants.h"
#include "ExceptionT.h"
#include "FEManagerT.h"

/* constructor */

using namespace Tahoe;

NLSolver_LS::NLSolver_LS(FEManagerT& fe_manager, int group):
	NLSolver(fe_manager, group)
{
	ifstreamT& in = fFEManager.Input();
	
	/* read parameters */
	in >> fSearchIterations;
	in >> fOrthogTolerance;
	in >> fMaxStepSize;

	/* mininum search iterations > 0 */
	fSearchIterations = (fSearchIterations != 0 &&
	                     fSearchIterations < 3) ? 3 : fSearchIterations;

	/* print parameters */
	ostream& out = fFEManager.Output();
	out << " Maximum number of line search iterations. . . . = " << fSearchIterations << '\n';
	out << " Line search orthoginality tolerance . . . . . . = " << fOrthogTolerance  << '\n';
	out << " Maximum update step size. . . . . . . . . . . . = " << fMaxStepSize      << endl;
	
	/* checks */
	if (fSearchIterations < 0)  throw ExceptionT::kBadInputValue;
	if (fOrthogTolerance > 1.0) throw ExceptionT::kBadInputValue;
	if (fMaxStepSize      < 0)  throw ExceptionT::kBadInputValue;
	
	/* allocate space for history */
	fSearchData.Dimension(fSearchIterations, 2);
	
	/* set console */
	iAddVariable("line_search_iterations", fSearchIterations);
	iAddVariable("line_search_tolerance", fOrthogTolerance);
	iAddVariable("max_step_size", fMaxStepSize);
}

/* form and solve the equation system */
double NLSolver_LS::SolveAndForm(bool newtangent)
{		
	/* form the stiffness matrix */
	if (newtangent)
	{
		fLHS->Clear();
		fFEManager.FormLHS(Group());
	}
	
	/* store residual */
	fR = fRHS;
		 		
	/* solve equation system */
	if (!fLHS->Solve(fRHS)) throw ExceptionT::kBadJacobianDet;

	/* apply update to system */
	Update(fRHS, &fR);
								
	/* compute new residual */
	fRHS = 0.0;
	fFEManager.FormRHS(Group());

	/* combine residual magnitude with update magnitude */
	/* e = a1 |R| + a2 |delta_d|                        */
	//not implemented!
			
	return fRHS.Magnitude();
}

/* console */
bool NLSolver_LS::iDoVariable(const StringT& variable, StringT& line)
{
	/* inherited */
	bool result = NLSolver::iDoVariable(variable, line);
	if (result)
	{
		/* need to reallocate */
		if (variable == "line_search_iterations")
			fSearchData.Dimension(fSearchIterations, 2);
	}
	return result;
}

/*************************************************************************
* Protected
*************************************************************************/

void NLSolver_LS::Update(const dArrayT& update, const dArrayT* residual)
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
		
		/* out of range -> contract */
		if (s_new > fMaxStepSize || s_new < 0.0)
			s_new = 0.5*(s_a + s_b);

		/* update and compute test value */				
		G_new = GValue(s_new);
		fSearchData(count, 0) = s_new;
		fSearchData(count, 1) = G_new;

		/* secant search */
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
		int best = 0;
		if (fabs(fSearchData(best,0)) < kSmall) best = 1; // skip zero step
		double G_best = fabs(fSearchData(best,1));
		for (int i = best + 1; i < count; i++)
		{
			double G_test = fabs(fSearchData(i,1));
			if (G_test < G_best)
			{
				G_best = G_test;
				best = i;
			}
		}
	
		/* set to "best" */
		GValue(fSearchData(best,0));
	}

	/* write results */
	cout << " LS: " << count << setw(kDoubleWidth) << s_current << " | ";
} 	

/*************************************************************************
* Private
*************************************************************************/

/* return the line search weight function for the given step size */
double NLSolver_LS::GValue(double step)
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
		cout << "\n NLSolver_LS::GValue: caught exception: " << error << endl;
		throw error;
	}

	return InnerProduct(fUpdate, fRHS);
}
