/* $Id: FDSV_KStV3D.h,v 1.1 2002-10-05 00:49:21 thao Exp $ */
/* created:   TDN (5/31/2001) */

#ifndef _FD_SV_KStV_3D_H_
#define _FD_SV_KStV_3D_H_

#include "FDSimoViscoBaseT.h"

class ifstreamT;
class IsotropicT;

namespace Tahoe {

/** baseclass for standard solid Kirchhoff St. Venant constitutive models 
 * constitutive law */
class FDSV_KStV3D: public FDSimoViscoBaseT
{
	public:
	
	/*constructor*/
	FDSV_KStV3D(ifstreamT& in, const FiniteStrainT& element);
		
	/*print parameters*/
	void Print(ostream& out) const;
	void PrintName(ostream& out) const;

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
#endif  /* _FD_SV_KStV_3D_H_ */
