/* $Id: DPSSKStV.h,v 1.11.4.2 2004-06-09 23:17:54 paklein Exp $ */
/* created: myip (06/01/1999) */
#ifndef _DP_SS_KSTV_H_
#define _DP_SS_KSTV_H_

/* base classes */
#include "SSSolidMatT.h"
#include "IsotropicT.h"
#include "HookeanMatT.h"
#include "DPSSLinHardT.h"

namespace Tahoe {

class DPSSKStV: public SSSolidMatT,
				public IsotropicT,
				public HookeanMatT,
				public DPSSLinHardT
{
  public:

	/* constructor */
	DPSSKStV(ifstreamT& in, const SSMatSupportT& support);

	/* initialization */
	virtual void Initialize(void);

	/* form of tangent matrix (symmetric by default) */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	/* update internal variables */
	virtual void UpdateHistory(void);

	/* reset internal variables to last converged solution */
	virtual void ResetHistory(void);

	/** \name spatial description */
	/*@{*/
	/** spatial tangent modulus */
	virtual const dMatrixT& c_ijkl(void);
	
	/** Cauchy stress */
	virtual const dSymMatrixT& s_ij(void);

	/** return the pressure associated with the last call to 
	 * SolidMaterialT::s_ij. See SolidMaterialT::Pressure
	 * for more information. */
	virtual double Pressure(void) const { return fStress.Trace()/3.0; };
	/*@}*/

	/* returns the strain energy density for the specified strain */
	virtual double StrainEnergyDensity(void);

	/* returns the number of variables computed for nodal extrapolation
	 * during for element output, ie. internal variables */
	virtual int  NumOutputVariables(void) const;
	virtual void OutputLabels(ArrayT<StringT>& labels) const;
	virtual void ComputeOutput(dArrayT& output);

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& list_name) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

protected:

	/* set modulus */
 	virtual void SetModulus(dMatrixT& modulus); 
 
private:
  
  	/* return values */
  	dSymMatrixT	fStress;
  	dMatrixT	fModulus;

};

} // namespace Tahoe 
#endif /* _DP_SS_KSTV_H_ */
