/* $Id: YoonAllen2DT.h,v 1.2.2.1 2002-06-27 18:02:38 cjkimme Exp $ */
/* created: cjkimme (05/28/2002) */

#ifndef _YOON_ALLEN_2D_T_H_
#define _YOON_ALLEN_2D_T_H_

/* base class */
#include "SurfacePotentialT.h"

/* forward declarations */

namespace Tahoe {

class ifstreamT;

/** cohesive law from Yoon and Allen. This model is
 ** described in IJF 96, 55-74. 
 **/
class YoonAllen2DT: public SurfacePotentialT
{
public:

	/** constructor */
	YoonAllen2DT(ifstreamT& in, const double &fTimeStep);

	virtual void InitStateVariables(ArrayT<double>& state);

	/** return the number of state variables needed by the model */
	int NumStateVariables(void) const;

	/** dissipated energy */
	virtual double FractureEnergy(const ArrayT<double>& state);

	/** potential energy */
	virtual double Potential(const dArrayT& jump_u, const ArrayT<double>& state);
	
	/** surface traction. Internal variables are integrated over the current
	 * time step. */	
	virtual const dArrayT& Traction(const dArrayT& jump_u, ArrayT<double>& state, const dArrayT& sigma);

	/** tangent stiffness */
	virtual const dMatrixT& Stiffness(const dArrayT& jump_u, const ArrayT<double>& state, const dArrayT& sigma);

	/** surface status */
	virtual StatusT Status(const dArrayT& jump_u, const ArrayT<double>& state);

	/** write model name to output */
	virtual void PrintName(ostream& out) const;

	/** write model parameters */
	virtual void Print(ostream& out) const;

	/** return the number of output variables. returns 0 by default. */
	virtual int NumOutputVariables(void) const;

	/** return labels for the output variables.
	 * \param labels returns with the labels for the output variables. Space is
	 *        allocate by the function. Returns empty by default. */
	virtual void OutputLabels(ArrayT<StringT>& labels) const;

	/** compute the output variables.
	 * \param destination of output values. Allocated by the host code */
	virtual void ComputeOutput(const dArrayT& jump, const ArrayT<double>& state, 
		dArrayT& output);

	virtual bool NeedsNodalInfo(void);
	virtual int NodalQuantityNeeded(void);
//        virtual double ComputeNodalValue(const dArrayT &);
//	virtual void UpdateStateVariables(const dArrayT &, ArrayT<double> &);
	virtual int ElementGroupNeeded(void);

protected:

	/** return true if the potential has compatible (type and sequence)
	 * nodal output - FALSE by default */
	virtual bool CompatibleOutput(const SurfacePotentialT& potential) const;
	
private:

	/* traction potential parameters */
	double fsigma_0; /**< initiation stress */
	double fd_c_n;     /**< characteristic normal length scale */
	double fd_c_t;     /**< characteristic tangential length scale */
	
	/* moduli */
	double fE_infty; /**< Asymptotic modulus of cohesive zone */
	int fNumRelaxTimes;
	dArrayT fE_t; /**< transient modulus with exponential time decay*/
	dArrayT ftau; /**< time constant for decay */
	dArrayT fexp_tau; /**< exponentiations of the timestep over the time constants */
	
	/* damage evolution law parameters */
	double falpha_exp, flambda_exp;
	double falpha_0, flambda_0;
	
	/* penetration stiffness */
	double fpenalty; /**< stiffening multiplier */
	double fK;       /**< penetration stiffness calculated as a function of penalty
	                  * and the initial stiffness of the cohesive potential */
	const double& fTimeStep;
};

} // namespace Tahoe 
#endif /* _YOON_ALLEN_2D_T_H_ */
