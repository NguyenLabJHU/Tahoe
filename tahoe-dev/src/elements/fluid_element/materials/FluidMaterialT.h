/* $Header: /home/regueiro/tahoe_cloudforge_repo_snapshots/development/src/elements/fluid_element/materials/FluidMaterialT.h,v 1.5 2016-09-15 15:46:37 tahoe.vickynguyen Exp $ */
/* created: tdnguye (07/12/2006) */
#ifndef _FLUID_MATERIALT_H_
#define _FLUID_MATERIALT_H_

/* base class */
#include "ContinuumMaterialT.h"

/* direct members */
#include "dMatrixT.h"
#include "dSymMatrixT.h"
#include "dArrayT.h"

namespace Tahoe {

/* forward declarations */
class ifstreamT;
class FluidT;
class FluidMatSupportT;

/** interface for materials for fluid */
class FluidMaterialT: public ContinuumMaterialT
{
public:

	/** constructor */
	FluidMaterialT(void);

	/** set support */
	void SetFluidMatSupport(const FluidMatSupportT* support);

	/* form of tangent matrix (symmetric by default) */
	GlobalT::SystemTypeT TangentType(void) const;
		
	/** \name parameters at the current field point */
	/*@{*/
	/** viscosity */
	virtual const dMatrixT& c_ijkl(void);

	/** stress */
	virtual const dSymMatrixT& s_ij(void);

	double Density(void) const;
	double Shear_Modulus(void) const;

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** describe the parameters needed by the interface */
	virtual void DefineParameters(ParameterListT& list) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

protected:

	/** support for fluid materials */
	const FluidMatSupportT* fFluidMatSupport;

	/** \name parameters */
	/*@{*/
	double   fDensity;
	double   fMu;
	/*@}*/

	/** pressure flux variation return value */
	dSymMatrixT fStress;  
	dSymMatrixT fStrainRate;

	/** conductivity variation return value */
	dMatrixT fModulus;  
	/*@}*/

	/** FOR DEBUGGING PURPOSES ONLY */
	void WriteCallLocation( char* loc ) const;
};

/* returns the density */
inline double FluidMaterialT::Density(void) const { return fDensity; }
inline double FluidMaterialT::Shear_Modulus(void) const { return fMu; }

} // namespace Tahoe 
#endif /* _FLUID_MATERIALT_H_ */
