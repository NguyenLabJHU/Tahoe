/* $Id: DPSSKStV.h,v 1.7 2002-07-02 19:56:09 cjkimme Exp $ */
/* created: myip (06/01/1999)                                  */
/* $Id: DPSSKStV.h,v 1.7 2002-07-02 19:56:09 cjkimme Exp $ */
/* created: myip (06/01/1999)                                             */

#ifndef _DP_SS_KSTV_H_
#define _DP_SS_KSTV_H_

/* base classes */
#include "SSStructMatT.h"
#include "IsotropicT.h"
#include "HookeanMatT.h"
#include "DPSSLinHardT.h"


namespace Tahoe {

class DPSSKStV: public SSStructMatT,
				public IsotropicT,
				public HookeanMatT,
				public DPSSLinHardT
{
  public:

	/* constructor */
	DPSSKStV(ifstreamT& in, const SmallStrainT& element);

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
	virtual const dMatrixT& cdisc_ijkl(void);
  	
	/* stress */
	virtual const dSymMatrixT& s_ij(void);

	/* returns the strain energy density for the specified strain */
	virtual double StrainEnergyDensity(void);

	/* returns the number of variables computed for nodal extrapolation
	 * during for element output, ie. internal variables */
	virtual int  NumOutputVariables(void) const;
	virtual void OutputLabels(ArrayT<StringT>& labels) const;
	virtual void ComputeOutput(dArrayT& output);

        /*
         * Test for localization using "current" values for Cauchy
         * stress and the spatial tangent moduli. Returns 1 if the
         * determinant of the acoustic tensor is negative and returns
         * the normal for which the determinant is minimum. Returns 0
         * of the determinant is positive.
         */
	 int IsLocalized(dArrayT& normal);

protected:

	/* set modulus */

 	virtual void SetModulus(dMatrixT& modulus); 
         int loccheck;
 
  private:
  
  	/* return values */
  	dSymMatrixT	fStress;
  	dMatrixT	fModulus;
        dMatrixT        fModulusdisc;

};

} // namespace Tahoe 
#endif /* _DP_SS_KSTV_H_ */