/* $Id: Tijssens2DT.h,v 1.6 2002-02-18 19:09:43 cjkimme Exp $ */

#ifndef _TIJSSENS_2D_T_H_
#define _TIJSSENS_2D_T_H_

/* base class */
#include "SurfacePotentialT.h"

/* forward declarations */
class ifstreamT;
class FEManagerT;

/**Rate dependence of traction as a function of the rate of change
 * of the gap vector as described by Tijssens, et. al in Mech. Mat. 32 19-35. 
 * Crazing of polymers is modeled by the state variables.
 */
class Tijssens2DT: public SurfacePotentialT
{
public:

	/** constructor.
	 * \param time_step reference to the current time step */
	Tijssens2DT(ifstreamT& in, double time_step,FEManagerT& FE_Manager);

	/** return the number of state variables needed by the model.
	 * Need to store the opening displacement from the previous
	 * time increment. */
	int NumStateVariables(void) const;

	/** dissipated energy */
	virtual double FractureEnergy(const ArrayT<double>& state);

	/** potential energy */
	virtual double Potential(const dArrayT& jump_u, const ArrayT<double>& state);
	
	/** surface traction. Internal variables are integrated over the current
	 * time step. */	
	virtual const dArrayT& Traction(const dArrayT& jump_u, ArrayT<double>& state);

	/** tangent stiffness */
	virtual const dMatrixT& Stiffness(const dArrayT& jump_u, const ArrayT<double>& state);

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

	/** For Tijssens2DT, returns true to compute nodal tractions. */
	virtual bool NeedsNodalInfo(void);
	virtual int NodalQuantityNeeded(void);
        virtual double ComputeNodalValue(const dArrayT &);
	virtual void UpdateStateVariables(const dArrayT &, ArrayT<double> &);
	virtual int ElementGroupNeeded(void);

protected:

	/** return true if the potential has compatible (type and sequence)
	 * nodal output - FALSE by default */
	virtual bool CompatibleOutput(const SurfacePotentialT& potential) const;
	
private:

	//bool initiationQ(const ArrayT<double>&);

	/** the time step */
	double fTimeStep;
	FEManagerT& fFEManager;

	/* traction rate parameters */
	double fk_t0; /* initial tangential stiffness */
	double fk_n;     /* normal stiffness */
	double fc_1;     /* tangential stiffness decay parameter */
	double fDelta_n_ccr; /* tangential stiffness decay parameter */

	/* craze initiation parameters */
	double fA_0, fA, fQ_A; /* fA = fA_0 exp(fQ_A/fTemp) */  
	double fB_0, fB, fQ_B; /* fB = fB_0 exp(fQ_B/fTemp) */

	/* crazing state variables' parameters */
	double fGamma_0; /* tangential rate constant */
	double fDelta_0; /* normal rate constant */
	double fsigma_c; /* critical normal traction */
	double ftau_c; /* critical tangential traction */
	double fastar; /* Material parameter */
	double ftemp; /* Temperature */
//	double fY; /* Bulk yield strength */
	int fGroup; /* element group to obtain hydrostatic stress from */
	double fSteps; /* number of steps for k_n to go to 0 after failure */

};

#endif /* _TIJSSENS_2D_T_H_ */

