#ifndef _GRAD_J2_SS_NONLIN_HARD_H_
#define _GRAD_J2_SS_NONLIN_HARD_H_

/* base classes */
#include "SSStructMatT.h"
#include "IsotropicT.h"
#include "HookeanMatT.h"

/* direct members */
#include "dSymMatrixT.h"
#include "dMatrixT.h"
#include "dArrayT.h"
#include "LocalArrayT.h"
#include "ArrayT.h"

#include "GlobalT.h"

namespace Tahoe {

class ElementCardT;
class ifstreamT;

class GradJ2SSNonlinHard: public SSStructMatT,
	    public IsotropicT,
	    public HookeanMatT
{
public:

	/* constructor */
	GradJ2SSNonlinHard(ifstreamT& in, const SmallStrainT& element);

	/* initialization */
	virtual void Initialize(void);

	/* form of tangent matrix (symmetric by default) */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/* update internal variables */
	virtual void UpdateHistory(void);

	/* reset internal variables to last converged solution */
	virtual void ResetHistory(void);

	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;
	
	/* modulus */
	virtual const dMatrixT& c_ijkl(void);
	
	/* stress */
	virtual const dSymMatrixT& s_ij(void);

	/* returns the strain energy density for the specified strain */
	virtual double StrainEnergyDensity(void);

	/* returns the number of variables computed for nodal extrapolation
	 * during for element output, ie. internal variables */
	virtual int NumOutputVariables(void) const;
	virtual void OutputLabels(ArrayT<StringT>& labels) const;
	virtual void ComputeOutput(dArrayT& output);

protected:

	/* set modulus */
	virtual void SetModulus(dMatrixT& modulus);

	/* returns elastic strain */
	virtual const dSymMatrixT& ElasticStrain(const dSymMatrixT& totalstrain,
		const ElementCardT& element, int ip);
			
	/* solve for the state at selected ip */
	void SolveState(ElementCardT& element);

	/* return pointers to new element objects constructed with the data from elements */
	void AllocateAllElements();

	/* indexes to access internal variable (scalars) array */
	enum InternalVariablesT {kIsotHard    = 0,  // isotropic hardening
				 kNlIsotHard  = 1,  // nonlocal isotropic hardening
                                 kdelLmbda    = 2,  // consistency parameter
			         kYieldCrt    = 3,  // yield criteria
				 kIsotHard0   = 4,  // isotropic hardening at previous iteration
				 kNlIsotHard0 = 5}; // nonlocal syisotropic hardening at previous iteration

	/* returns the value of the yield function given the relative stress and isotropic hardening */
	double YieldCondition(const dSymMatrixT& relstress, double isotropic) const;

private:
	/* status flags */
	enum LoadingStatusT {kIsPlastic = 0,
                             kIsElastic = 1,
                                 kReset = 3}; // indicator not to repeat update

	/* load element data for the specified integration point */
	void LoadData(const ElementCardT& element, int fCurrIP);

	/* computes the increment in the plasticity parameter */
	void IncrementPlasticParameter(double& varLambda, const ElementCardT& element, int ip);

	/* computes the increments in the stress and internal variables */
	void IncrementState(const double& varLambda, const ElementCardT& element, int ip);

	/* computes the unit normal and the yield condition */
	void UpdateState(const ElementCardT& element, int ip);

	/* computes the consistent tangent moduli */
	void TangentModuli(const ElementCardT& element, int ip);

	/* check convergence of solution for all ip of element */
	bool CheckElementState(const ElementCardT& element);

	/* returns the laplacian of the field passed */
	dArrayT Laplacian(const dArrayT& ip_field, int field_length);

protected:

	/* status flag */
	const GlobalT::StateT& fStatus;

	/* element level internal variables at current time step*/
	dSymMatrixT fStress;        //stress
	dSymMatrixT fPlstStrn;      //plastic strain
	dSymMatrixT fUnitNorm;      //unit normal to the stress surface
	dSymMatrixT fKineHard;      //stress surface "center", kinematic hardening
	dSymMatrixT fNlKineHard;      //stress surface "center", kinematic hardening
	dArrayT     fInternal;      //internal variables

	/* element level internal variables at previous time step*/
	dSymMatrixT fStress_n;      //stress
	dSymMatrixT fPlstStrn_n;    //plastic strain
	dSymMatrixT fUnitNorm_n;    //unit normal to the stress surface
	dSymMatrixT fKineHard_n;    //stress surface "center", kinematic hardening
	dSymMatrixT fNlKineHard_n;    //stress surface "center", kinematic hardening
	dArrayT     fInternal_n;    //internal variables

private:

	/* number of integration points */
	int fNumIP;

	/* number of nodes per element */
	int fNumNodes;

	/* material parameters **/
	double fmu;

    	/* material constants */
        double yield;
        double k1, k2, k3, k4;
	double c1, c2;

	/* return values */
	dSymMatrixT	fElasticStrain;
	dMatrixT	fModulus;
	dMatrixT	fModuliCorr;

        /* general workspaces */
	dSymMatrixT     fRelStress;
        dSymMatrixT     fsymmatx1;
        dMatrixT        fmatx1;
        dMatrixT        fmatx2;
        dMatrixT        fmatx3;
	dMatrixT        ftnsr1;
};

} // namespace Tahoe
#endif /* _GRAD_J2_SS_NONLIN_HARD_H_ */
