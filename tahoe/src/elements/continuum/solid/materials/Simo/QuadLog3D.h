/* $Id: QuadLog3D.h,v 1.7 2002-10-20 22:48:50 paklein Exp $ */
/* created: paklein (06/27/1997)                                          */
/* Hyperelastic material governed by quadratic logarithmic potential.     */

#ifndef _QUAD_LOG_3D_H_
#define _QUAD_LOG_3D_H_

/* base classes */
#include "FDStructMatT.h"
#include "IsotropicT.h"
#include "SpectralDecompT.h"


namespace Tahoe {

class QuadLog3D: public FDStructMatT, public IsotropicT
{
public:

	/* constructor */
	QuadLog3D(ifstreamT& in, const FiniteStrainT& element);
	
	/* print parameters */
	virtual void Print(ostream& out) const;
	virtual void PrintName(ostream& out) const;

	/** \name spatial description */
	/*@{*/
	/** spatial tangent modulus */
	virtual const dMatrixT& c_ijkl(void);

	/** Cauchy stress */
	virtual const dSymMatrixT& s_ij(void);

	/** return the pressure associated with the last call to 
	 * StructuralMaterialT::s_ij. See StructuralMaterialT::Pressure
	 * for more information. */
	virtual double Pressure(void) const { return fStress.Trace()/3.0; };
	/*@}*/

	/* material description */
	virtual const dMatrixT& C_IJKL(void); // material tangent moduli
	virtual const dSymMatrixT& S_IJ(void); // PK2 stress
//TEMP - no reason to use these in total Lagrangian formulation.
//       calls to these write error message and throw ExceptionT::xception

	/* strain energy density */
	virtual double StrainEnergyDensity(void);

protected:

	/* computation routines */
	void ComputeModuli(const dSymMatrixT& b, dMatrixT& moduli);
	void ComputeCauchy(const dSymMatrixT& b, dSymMatrixT& cauchy);
	double ComputeEnergy(const dArrayT& loge);

	/* compute logarithmic stretches from the given eigenvalues */
	void LogStretches(const dArrayT& eigs);

protected:

	/* spectral decomposition solver */
	SpectralDecompT fSpectral;

	/* left stretch */
	dSymMatrixT fb;

	/* return values */
	dSymMatrixT	fStress;
	dMatrixT	fModulus;

	/* deviatoric operator */
	dSymMatrixT fDevOp3;

	/* spectral decomposition */
	dArrayT	fEigs;  //principal value of b
	dArrayT	floge;  //logarithmic stretches
	dArrayT	fBeta;  //principal stresses
	dSymMatrixT fEigMod;//modulus in principal stretches
};

} // namespace Tahoe 
#endif /* _QUAD_LOG_3D_H_ */
