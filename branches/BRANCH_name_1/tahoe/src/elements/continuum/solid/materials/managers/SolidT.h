/* $Id: SolidT.h,v 1.15.2.1 2002-06-27 18:03:29 cjkimme Exp $ */
/* created: paklein (03/10/2001)                                          */

#ifndef _MATERIAL_T_H_
#define _MATERIAL_T_H_

/* forward declarations */
#include "ios_fwd_decl.h"


namespace Tahoe {

class SolidT
{
public:

	/* solid material types */
	enum SolidT {
         kSSKStV = 1,			
         kFDKStV = 2,			
        kSSCubic = 3,			
        kFDCubic = 4,			
        kSimoIso = 5,
        kQuadLog = 6,
   kQuadLogOgden = 7,
       kJ2SSKStV = 8,
         kJ2Simo = 9,
           kJ2QL = 10,
       kDPSSKStV = 11,
         kLJTr2D = 12,
       kLJFCC111 = 13,
         kFCCEAM = 14,
kmodCauchyBornDC = 15,
            kVIB = 16,
     kIsoVIBSimo = 17,
    kIsoVIBOgden = 18,
   kIsoVIBSimoJ2 = 19,
kThermoViscoPlastic = 30,
kPovirk2D = 31,
       kHyperEVP = 40,
        kBCJHypo = 45,
kBCJHypoIsoDmgKE = 46,
kBCJHypoIsoDmgYC = 47,
    kFDXtalElast = 49,
   kLocXtalPlast = 50,
 kLocXtalPlast_C = 51,
   kGrdXtalPlast = 52,
 kLocXtalPlastFp = 55,
kLocXtalPlastFp_C = 56,
 kGrdXtalPlastFp = 57,
   kOgdenViscVIB = 60,
     kSSStandard = 61,
     kFDStandard = 62,
     kABAQUS_BCJ = 80,
kABAQUS_VUMAT_BCJ = 90
		};
	/* stream extraction operator */ 
	friend istream& operator>>(istream& in, SolidT::SolidT& code);

/* 2D types */
#if 0
const int kSWDC100         = 24;//improper CB material
const int kSWDC110         = 15;	//improper CB material
const int kD2VIB           = 23; // plane stress VIB + gradient terms
} // namespace Tahoe 
#endif

/* 3D types */
#if 0
const int kIsoVIB_X	    = 14; // remove
} // namespace Tahoe 
#endif
  
};

} // namespace Tahoe 
#endif // _MATERIAL_T_H_
