/* $Id: DetCheckT.h,v 1.11 2002-07-13 00:03:58 cfoster Exp $ */
/* created: paklein (09/11/1997) */

#ifndef _DETCHECK_T_H_
#define _DETCHECK_T_H_

#define sweepIncr 5         // should be an integral divisor of 360
#define numThetaChecks 72    // should be 360/sweepIncr and should be even
#define numPhiChecks  19      // should be 90/sweepIncr+1

# include "AutoArrayT.h"

namespace Tahoe {

/* forward declarations */
class dSymMatrixT;
class dMatrixT;
class dMatrixEXT;
class dArrayT;
class dTensor4DT;

/** class to support checks of loss of ellipticity.  \note this class does 
 * not dynamically allocate memory on construction */
class DetCheckT
{
public:

	/** constructor
	 * \param s_jl Cauchy stress
	 * \param c_ijkl spatial tangent modulus */
	DetCheckT(const dSymMatrixT& s_jl, const dMatrixT& c_ijkl);

	/** check ellipticity of tangent modulus.
	 * \return 1 if acoustic tensor isn't positive definite,
	 * and returns the normal to the surface of localization.
	 * returns 0, otherwise */
	int IsLocalized(dArrayT& normal);

	/** check ellipticity of tangent modulus using closed form algorithm
	 * taken from R.A.Regueiro's SPINLOC.
	 * \return 1 if acoustic tensor isn't positive definite,
	 * and returns the normal to the surface of localization.
	 * returns 0, otherwise */
	int IsLocalized_SS(dArrayT& normal);

private:

	/** closed-form check for localization, assuming plane strain conditions.
	 * Taken from R.A.Regueiro's SPINLOC.
	 * 1 is 11
	 * 2 is 22
	 * 3 is 12
	 * angle theta subtends from the x1 axis to the band normal */
	int SPINLOC_localize(double *c__, double *thetan, int *loccheck);

	/*3D Small Strain check for localization */
	int DetCheck3D_SS(dArrayT& normal);

	/* auxiliary functions to DetCheck3D_SS */
	void FindApproxLocalMins(double detA [numThetaChecks] [numPhiChecks],
				 int localmin [numThetaChecks] [numPhiChecks], dTensor4DT& C);
	dArrayT ChooseNewNormal(dArrayT& prevnormal, dMatrixEXT& J);
	dArrayT ChooseNormalFromNormalSet(AutoArrayT <dArrayT> &normalSet, dTensor4DT &C);

	/* 2D determinant check function */
	int DetCheck2D(dArrayT& normal);

	/* compute coefficients of det(theta) function */
	void ComputeCoefficients(void);

	/* determinant function and derivatives */
	double det(double t) const;
	double ddet(double t) const;
	double dddet(double t) const;

private:

	const dSymMatrixT& 	fs_jl;	/* Cauchy stress          */
	const dMatrixT&		fc_ijkl;/* spatial tangent moduli */

	double phi2, phi4;	/* phase shifts */
	double A0, A2, A4;	/* amplitudes   */
};

} // namespace Tahoe 
#endif /* _DETCHECK_T_H_ */