/* $Id: LinearSolver.cpp,v 1.4 2002-07-02 19:57:11 cjkimme Exp $ */
/* created: paklein (05/30/1996) */

#include "LinearSolver.h"
#include "FEManagerT.h"

/* constructor */

using namespace Tahoe;

LinearSolver::LinearSolver(FEManagerT& fe_manager, int group):
	SolverT(fe_manager, group),
	fFormLHS(1)
{

}

/* signal new reconfigured system */
void LinearSolver::Initialize(int tot_num_eq, int loc_num_eq, int start_eq)
{
	/* inherited */
	SolverT::Initialize(tot_num_eq, loc_num_eq, start_eq);
	
	/* flag to reform LHS */
	fFormLHS = 1;
}

/* solve the current step */
SolverT::SolutionStatusT LinearSolver::Solve(int)
{
	try {
	/* initialize */
	fRHS = 0.0;
			
	/* form the residual force vector */
	fFEManager.FormRHS(Group());
					
	/* solve equation system */
	if (fFormLHS)
	{
		/* initialize */
		fLHS->Clear();
	
		/* form the stiffness matrix */
		fFEManager.FormLHS(Group());
				
		/* flag not to reform */
		fFormLHS = 0;
	}

	/* determine update vector */
	if (!fLHS->Solve(fRHS)) throw eBadJacobianDet;
	fNumIteration = 1;

	/* update displacements */
	fFEManager.Update(Group(), fRHS);		
			
	/* relaxation */
	GlobalT::RelaxCodeT relaxcode = fFEManager.RelaxSystem(Group());
				
	/* relax for configuration change */
	if (relaxcode == GlobalT::kRelax) fFormLHS = 1;
			//NOTE: NLSolver calls "fFEManager.Reinitialize()". Should this happen
			//      here, too? For statics, should also reset the structure of
			//      global stiffness matrix, but since EFG only breaks connections
			//      and doesn't make new ones, this should be OK for now. PAK (03/04/99)
			
	/* trigger set of new equations */
	if (relaxcode == GlobalT::kReEQ ||
	    relaxcode == GlobalT::kReEQRelax)
		fFEManager.SetEquationSystem(Group());

	return kConverged;
	} /* end try */
	
	/* not OK */
	catch (int exception)
	{
		cout << "\n LinearSolver::Solve: caught exception: " 
		     << fFEManager.Exception(exception) << endl;
		return kFailed;
	}
}
