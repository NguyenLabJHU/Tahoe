/* $Id: SolidMatList3DT.cpp,v 1.24.2.1 2002-12-08 23:47:36 paklein Exp $ */
/* created: paklein (02/14/1997) */
#include "SolidMatList3DT.h"
#include "MaterialsConfig.h"
#include "fstreamT.h"

/* 3D material type codes */
#include "SSKStV.h"
#include "FDKStV.h"
#include "SSCubicT.h"
#include "FDCubicT.h"
#include "QuadLog3D.h"
#include "SimoIso3D.h"
#include "J2Simo3D.h"
#include "DPSSKStV.h"
#include "J2SSKStV.h"
#include "J2QLLinHardT.h"
#include "QuadLogOgden3DT.h"
#include "FossumSSIsoT.h"
#include "FDCrystalElast.h"
#include "tevp3D.h"
#include "LocalJ2SSNonlinHard.h"
#include "GradJ2SSNonlinHard.h"
#include "ABAQUS_BCJ.h"
#include "ABAQUS_VUMAT_BCJ.h"

#ifdef EAM_MATERIAL
#include "EAMFCC3DMatT.h"
#endif

#ifdef MODCBSW_MATERIAL
#include "ModCB3DT.h"
#endif

#ifdef VIB_MATERIAL
#include "VIB3D.h"
#include "IsoVIB3D.h"
#include "J2IsoVIB3DLinHardT.h"
#include "OgdenIsoVIB3D.h"
#endif

#ifdef SIMO_HOLZAPFEL_MATERIAL
#include "SV_NeoHookean3D.h"
#include "SSSV_KStV3D.h"
#include "FDSV_KStV3D.h"
#endif

#ifdef REESE_GOVINDJEE_MATERIAL
#include "RG_NeoHookean3D.h"
#endif

#ifdef PLASTICITY_CRYSTAL_MATERIAL
#include "LocalCrystalPlast.h"
#include "LocalCrystalPlast_C.h"
#include "GradCrystalPlast.h"
#include "LocalCrystalPlastFp.h"
#include "LocalCrystalPlastFp_C.h"
#include "GradCrystalPlastFp.h"
#endif

#ifdef PLASTICITY_MACRO_MATERIAL
#include "HyperEVP3D.h"
#include "BCJHypo3D.h"
#include "BCJHypoIsoDamageKE3D.h"
#include "BCJHypoIsoDamageYC3D.h"
#endif

using namespace Tahoe;

/* constructors */
SolidMatList3DT::SolidMatList3DT(int length, const StructuralMatSupportT& support):
	StructuralMatListT(length, support)
{

}

/* read material data from the input stream */
void SolidMatList3DT::ReadMaterialData(ifstreamT& in)
{
	const char caller[] = "SolidMatList3DT::ReadMaterialData";

	int i, matnum;
	MaterialT::SolidT matcode;
	try {

	/* read material data */
	for (i = 0; i < fLength; i++)
	{
		in >> matnum; matnum--;
		in >> matcode;
		
		/* checks */
		if (matnum < 0  || matnum >= fLength) throw ExceptionT::kBadInputValue;

		/* repeated material number */
		if (fArray[matnum] != NULL)
			ExceptionT::BadInputValue(caller, "repeated material number: %d", matnum + 1);
		
		/* add to the list of materials */
		switch (matcode)
		{
			case kLJTr2D:
			case kLJFCC111:
			{
				ExceptionT::BadInputValue(caller, "material is 2D only: %d", matcode);
			}
			case kSSKStV:
			{
				/* check */
				if (!fSSMatSupport) Error_no_small_strain(cout, matcode);
	
				fArray[matnum] = new SSKStV(in, *fSSMatSupport);
				break;
			}
			case kFDKStV:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new FDKStV(in, *fFDMatSupport);
				break;
			}							
			case kSSCubic:
			{
				/* check */
				if (!fSSMatSupport) Error_no_small_strain(cout, matcode);
	
				fArray[matnum] = new SSCubicT(in, *fSSMatSupport);
				break;
			}
			case kFDCubic:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new FDCubicT(in, *fFDMatSupport);
				break;
			}
			case kSimoIso:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new SimoIso3D(in, *fFDMatSupport);
				break;
			}
			case kQuadLog:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new QuadLog3D(in, *fFDMatSupport);
				break;
			}
			case kQuadLogOgden:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new QuadLogOgden3DT(in, *fFDMatSupport);												
				break;
			}
			case kJ2SSKStV:
			{
				/* check */
				if (!fSSMatSupport) Error_no_small_strain(cout, matcode);
	
				fArray[matnum] = new J2SSKStV(in, *fSSMatSupport);
				fHasHistory = true;														
				break;
			}
			case kJ2Simo:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new J2Simo3D(in, *fFDMatSupport);
				fHasHistory = true;														
				break;
			}
			case kJ2QL:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new J2QLLinHardT(in, *fFDMatSupport);
				fHasHistory = true;														
				break;
			}
			case kDPSSKStV:
			{
				/* check */
				if (!fSSMatSupport) Error_no_small_strain(cout, matcode);
	
				fArray[matnum] = new DPSSKStV(in, *fSSMatSupport);
				fHasHistory = true;															
				break;
			}
			case kFCCEAM:
			{
#ifdef EAM_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new EAMFCC3DMatT(in, *fFDMatSupport);
				break;
#else
				ExceptionT::BadInputValue(caller, "EAM_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kmodCauchyBornDC:
			{
#ifdef MODCBSW_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new ModCB3DT(in, *fFDMatSupport, true);
				break;
#else
				ExceptionT::BadInputValue(caller, "MODCBSW_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kVIB:
			{
#ifdef VIB_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new VIB3D(in, *fFDMatSupport);
				fHasLocalizers = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "VIB_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kIsoVIBSimo:
			{
#ifdef VIB_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new IsoVIB3D(in, *fFDMatSupport);
				fHasLocalizers = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "VIB_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kIsoVIBOgden:
			{
#ifdef VIB_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new OgdenIsoVIB3D(in, *fFDMatSupport);
				fHasLocalizers = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "VIB_MATERIAL not enabled: %d", matcode);
#endif
			}	
			case kIsoVIBSimoJ2:
			{
#ifdef VIB_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new J2IsoVIB3DLinHardT(in, *fFDMatSupport);
				fHasLocalizers = true;
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "VIB_MATERIAL not enabled: %d", matcode);
#endif
			}	
			case kFossumSSIso:
			{
				/* check */
				if (!fSSMatSupport) Error_no_small_strain(cout, matcode);

				fArray[matnum] = new FossumSSIsoT(in, *fSSMatSupport);
				fHasHistory = true;
				break;
			}
			case kThermoViscoPlastic:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);
				
				fArray[matnum] = new tevp3D(in, *fFDMatSupport);
				fHasHistory = true;
				break;
			}
			case kHyperEVP:
			{
#ifdef PLASTICITY_MACRO_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new HyperEVP3D(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_MACRO_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kBCJHypo:
			{
#ifdef PLASTICITY_MACRO_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new BCJHypo3D(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_MACRO_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kBCJHypoIsoDmgKE:
			{
#ifdef PLASTICITY_MACRO_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new BCJHypoIsoDamageKE3D(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_MACRO_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kBCJHypoIsoDmgYC:
			{
#ifdef PLASTICITY_MACRO_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new BCJHypoIsoDamageYC3D(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_MACRO_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kFDXtalElast:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new FDCrystalElast(in, *fFDMatSupport);
				break;
			}
			case kLocXtalPlast:
			{
#ifdef PLASTICITY_CRYSTAL_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new LocalCrystalPlast(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_CRYSTAL_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kLocXtalPlast_C:
			{
#ifdef PLASTICITY_CRYSTAL_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new LocalCrystalPlast_C(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_CRYSTAL_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kGrdXtalPlast:
			{
#ifdef PLASTICITY_CRYSTAL_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new GradCrystalPlast(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_CRYSTAL_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kLocXtalPlastFp:
			{
#ifdef PLASTICITY_CRYSTAL_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new LocalCrystalPlastFp(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_CRYSTAL_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kLocXtalPlastFp_C:
			{
#ifdef PLASTICITY_CRYSTAL_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new LocalCrystalPlastFp_C(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_CRYSTAL_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kGrdXtalPlastFp:
			{
#ifdef PLASTICITY_CRYSTAL_MATERIAL
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new GradCrystalPlastFp(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "PLASTICITY_CRYSTAL_MATERIAL not enabled: %d", matcode);
#endif
			}
			case kLocJ2SSNlHard:
			{
				/* check */
				if (!fSSMatSupport) Error_no_small_strain(cout, matcode);
	
				fArray[matnum] = new LocalJ2SSNonlinHard(in, *fSSMatSupport);
				fHasHistory = true;														
				break;
			}
			case kGrdJ2SSNlHard:
			{
				/* check */
				if (!fSSMatSupport) Error_no_small_strain(cout, matcode);
	
				fArray[matnum] = new GradJ2SSNonlinHard(in, *fSSMatSupport);
				fHasHistory = true;														
				break;
			}
			case kABAQUS_BCJ:
			{
#ifdef __F2C__			
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new ABAQUS_BCJ(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "model requires f2c support: %d", kABAQUS_BCJ);
#endif /* __F2C__ */	
			}			
			case kABAQUS_VUMAT_BCJ:
			{
#ifdef __F2C__			
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new ABAQUS_VUMAT_BCJ(in, *fFDMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue(caller, "model requires f2c support: %d", kABAQUS_VUMAT_BCJ);
#endif /* __F2C__ */
			}			
			case kRGNeoHookean:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new RG_NeoHookean3D(in, *fFDMatSupport);
				fHasHistory = true;
				break;
			}
			case kSVNeoHookean:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new SV_NeoHookean3D(in, *fFDMatSupport);
				fHasHistory = true;
				break;
			}
			case kFDSVKStV:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new FDSV_KStV3D(in, *fFDMatSupport);
				fHasHistory = true;
				break;
			}
			case kSSSVKStV:
			{
				/* check */
				if (!fSSMatSupport) Error_no_small_strain(cout, matcode);

				fArray[matnum] = new SSSV_KStV3D(in, *fSSMatSupport);
				fHasHistory = true;
				break;
			}
//TEMP
#if 0
			case kIsoVIB_X:
			{
				/* check */
				if (!fFDMatSupport) Error_no_finite_strain(cout, matcode);

				fArray[matnum] = new IsoVIB3D_X(in, *fFDMatSupport);
				fHasLocalizers = true;
				break;
			}			
#endif
//TEMP
			default:
				ExceptionT::BadInputValue(caller, "unknown material code: %d", matcode);
		}

		/* safe cast since all structural */
		StructuralMaterialT* pmat = (StructuralMaterialT*) fArray[matnum];

		/* verify construction */
		if (!pmat) throw ExceptionT::kOutOfMemory;
		
		/* set thermal LTf pointer */
		int LTfnum = pmat->ThermalStrainSchedule();
		if (LTfnum > -1)
		{
			pmat->SetThermalSchedule(fStructuralMatSupport.Schedule(LTfnum));
			
			/* set flag */
			fHasThermal = true;
		}				

		/* perform initialization */
		pmat->Initialize();			
	}  } /* end try */

	catch (ExceptionT::CodeT error)
	{
		ExceptionT::Throw(error, caller, "exception constructing material %d, index %d, code %d",
			i+1, matnum+1, matcode);
	}
}

/* error messages */
void SolidMatList3DT::Error_no_small_strain(ostream& out, int matcode) const
{
	ExceptionT::BadInputValue("SolidMatList3DT::Error_no_small_strain", 
		"material %d requires a small strain element", matcode);
}

void SolidMatList3DT::Error_no_finite_strain(ostream& out, int matcode) const
{
	ExceptionT::BadInputValue("SolidMatList3DT::Error_no_small_strain", 
		"material %d requires a finite strain element", matcode);
}
