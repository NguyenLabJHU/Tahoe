/* $Id: SSSV_KStV2D.h,v 1.2.6.1 2002-10-28 06:49:08 paklein Exp $ */
/* created: TDN (5/31/2001) */
#ifndef _SS_SV_KStV_2D_H_
#define _SS_SV_KStV_2D_H_

#include "SSSimoViscoT.h"
#include "Material2DT.h"

namespace Tahoe {

/* forward declarations */
class ifstreamT;
class IsotropicT;

/** base class for standard solid Kirchhoff St. Venant constitutive models 
 * constitutive law */
class SSSV_KStV2D: public SSSimoViscoT, public Material2DT
{
	public:
	
	/*constructor*/
	SSSV_KStV2D(ifstreamT& in, const SSMatSupportT& support);
		
	/*print parameters*/
	void Print(ostream& out) const;
	void PrintName(ostream& out) const;

	virtual double StrainEnergyDensity(void);

        /* spatial description */ 
        const dMatrixT& c_ijkl(void); // spatial tangent moduli 
        const dSymMatrixT& s_ij(void); // Cauchy stress 
 
        /* material description */ 
        const dMatrixT& C_IJKL(void); // material tangent moduli 
        const dSymMatrixT& S_IJ(void); // PK2 stress 
 
        protected: 
	
	/*1/3*/
	const double fthird;
        /*strain energy potentials*/ 
	dArrayT fMu;
	dArrayT fKappa;

	/*strain*/
	dSymMatrixT fE;

        /*stress/modulus*/ 
        dMatrixT fModulus; 
        dSymMatrixT fStress; 
 
	/*relaxation times*/ 
        double ftauS; 
        double ftauB; 

        /* exp(-a* dt/tau)*/ 
        double falphaS; 
        double falphaB; 
        double fbetaS; 
        double fbetaB; 
};
}
#endif  /* _SS_SV_KStV_2D_H_ */
