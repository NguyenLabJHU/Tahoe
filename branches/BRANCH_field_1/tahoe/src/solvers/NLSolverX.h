/* $Id: NLSolverX.h,v 1.1.1.1.8.1 2002-04-25 01:37:48 paklein Exp $ */
/* created: paklein (08/25/1996) */

#ifndef _NL_SOLVER_X_H_
#define _NL_SOLVER_X_H_

/* base class */
#include "NLSolver.h"

/* forward declarations */
class CCSMatrixT;
class CCNSMatrixT;

/** nonlinear solver methods testbed */
class NLSolverX: public NLSolver
{
public:

	/* constructor */
	NLSolverX(FEManagerT& fe_manager, int group);

	/* generate the solution for the current time sequence */
	virtual void Run(void);

	/* form and solve the equation system - returns the magnitude of the
	 * residual */
	virtual double SolveAndForm(bool newtangent);

protected:

	/* relax system - reform tangent at newtancount intervals */
	virtual IterationStatusT Relax(int newtancount = 1);  	

	/* handlers */
	virtual IterationStatusT DoConverged(void);	
	virtual void DoNotConverged(void);

private:

	/* parameters */
	int fMaxNewTangents;  // max number of new tangents per step
	int fMaxTangentReuse; // max number iterations with same tangent
	int fMinFreshTangents;// number of new tan's at start of step (-1 for auto)
	int fCheckNegPivots; // automatic new tangent for negative pivots
	double fTrustTol;     // conv below which an extra 3 new tangents is allowed
	double fMinResRatio;  // min quasi-Newton reduction ratio: r_i+1/r_i

	/* runtime parameters */
	int fFormNewTangent; // 1 => new tangent, 0 => re-use
	int fNumNewTangent;

	/* store last update for recovery */
	dArrayT fLastUpdate;
	
	/* dynamically casted */
	CCSMatrixT*  pCCS;
	CCNSMatrixT* pCCNS;
};

#endif /* _NL_SOLVER_X_H_ */
