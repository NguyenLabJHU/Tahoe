/* $Id: J2Simo3D.h,v 1.6.2.1 2002-06-27 18:03:35 cjkimme Exp $ */
/* created: paklein (04/30/2001) */

#ifndef _J2_SIMO_3D_H_
#define _J2_SIMO_3D_H_

/* base classes */
#include "SimoIso3D.h"
//#include "J2SimoLinHardT.h"
#include "J2SimoC0HardeningT.h"

/* direct members */
#include "LocalArrayT.h"

//class J2Simo3D: public SimoIso3D, public J2SimoHardeningT

namespace Tahoe {

class J2Simo3D: public SimoIso3D, public J2SimoC0HardeningT
{
public:

	/** constructor */
	J2Simo3D(ifstreamT& in, const FiniteStrainT& element);

	/** form of tangent matrix (symmetric by default) */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/** update internal variables */
	virtual void UpdateHistory(void);

	/** reset internal variables to last converged solution */
	virtual void ResetHistory(void);

	/** print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;
	
	/** modulus */
	virtual const dMatrixT& c_ijkl(void);
	
	/** stress */
	virtual const dSymMatrixT& s_ij(void);

	/** incremental heat generation */
	virtual double IncrementalHeat(void);

	/** this model does generate heat */
	virtual bool HasIncrementalHeat(void) const { return true; };

	/** returns the strain energy density for the specified strain */
	virtual double StrainEnergyDensity(void);
	 	 	
	/** required parameter flags */
	virtual bool Need_F_last(void) const { return true; };

	/** returns the number of output variables */
	virtual int NumOutputVariables(void) const;

	/** returns labels for output variables */
	virtual void OutputLabels(ArrayT<StringT>& labels) const;

	/** compute output variables */
	virtual void ComputeOutput(dArrayT& output);

private:

	/** flag to indicate whether material supports thermal strains.
	 * Returns true. */
	virtual bool SupportsThermalStrain(void) const { return true; };

	/** compute F_mechanical and f_relative for the current step */
	void ComputeGradients(void);

private:

	/* deformation gradients */
	dMatrixT fFmech;
	dMatrixT ffrel;
	
	/* work space */
	dMatrixT fF_temp;
};

} // namespace Tahoe 
#endif /* _J2_SIMO_3D_H_ */
