/* $Id: LinearSolver.cpp,v 1.9 2003-08-14 05:31:46 paklein Exp $ */
/* created: paklein (05/30/1996) */
#include "LinearSolver.h"
#include "FEManagerT.h"

using namespace Tahoe;

/* constructors */
LinearSolver::LinearSolver(FEManagerT& fe_manager):
	SolverT(fe_manager),
	fFormLHS(1)
{
	SetName("linear_solver");
}

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
		/* unlock */
		fLHS_lock = kOpen;
	
		/* initialize */
		fLHS->Clear();
	
		/* form the stiffness matrix */
		fFEManager.FormLHS(Group(), GlobalT::kNonSymmetric);
				
		/* flag not to reform */
		fFormLHS = 0;

		/* lock */
		fLHS_lock = kLocked;
	}

	/* determine update vector */
	if (!fLHS->Solve(fRHS)) throw ExceptionT::kBadJacobianDet;
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
	catch (ExceptionT::CodeT exception)
	{
		cout << "\n LinearSolver::Solve: caught exception: " 
		     << ExceptionT::ToString(exception) << endl;
		return kFailed;
	}
}

/* signal time step change */
void LinearSolver::SetTimeStep(double dt)
{
	/* inherited */
	SolverT::SetTimeStep(dt);
	
	/* reform LHS matrix */
	fFormLHS = 1;
}
