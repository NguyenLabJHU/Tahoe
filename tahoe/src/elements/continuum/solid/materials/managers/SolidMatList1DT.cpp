/* $Id: SolidMatList1DT.cpp,v 1.10 2003-08-07 17:09:39 paklein Exp $ */
#include "SolidMatList1DT.h"
#include "SolidMatSupportT.h"
#include "fstreamT.h"

/* 1D material types codes */
/* Add small strain linear elastic material here */
#include "SSHookean1D.h"

#ifdef __DEVELOPMENT__
#include "DevelopmentElementsConfig.h"
#endif

#ifdef DORGAN_VOYIADJIS_MARIN_DEV
#include "GradJ2SS1D.h"
#endif

using namespace Tahoe;

/* constructor */
SolidMatList1DT::SolidMatList1DT(int length, const SolidMatSupportT& support):
	SolidMatListT(length, support)
{

}

/* read material data from the input stream */
void SolidMatList1DT::ReadMaterialData(ifstreamT& in)
{
	int i, matnum;
	SolidT::TypeT matcode;
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
		{
			cout << "\n SolidMatList1DT::ReadMaterialData: repeated material number: ";
			cout << matnum + 1 << endl;
			throw ExceptionT::kBadInputValue;
		}
		
		/* add to the list of matxxerials */
		switch (matcode)
		{
			case kSSKStV:
			{
				/* check */
				if (!fSSMatSupport) Error_no_small_strain(cout, matcode);
				fArray[matnum] = new SSHookean1D(in, *fSSMatSupport);
				break;
		  	}
			case kGradJ2SS:
			{
#ifdef DORGAN_VOYIADJIS_MARIN_DEV
				/* check */
				if (!fGradSSMatSupport) Error_no_small_strain(cout, matcode);

				fArray[matnum] = new GradJ2SS1D(in, *fGradSSMatSupport);
				fHasHistory = true;
				break;
#else
				ExceptionT::BadInputValue("SolidMatList1DT::ReadMaterialData", "DORGAN_VOYIADJIS_MARIN_DEV not enabled: %d", matcode);
#endif
			}
			default:
			{
				cout << "\n SolidMatList1DT::ReadMaterialData: unknown material code: ";
				cout << matcode << '\n' << endl;
				throw ExceptionT::kBadInputValue;
			}
		}

		/* safe cast since all structural */
		SolidMaterialT* pmat = (SolidMaterialT*) fArray[matnum];
		/* verify construction */
		if (!pmat) throw ExceptionT::kOutOfMemory;
		
		/* set thermal LTf pointer */
		int LTfnum = pmat->ThermalStrainSchedule();
		if (LTfnum > -1)
		{
			pmat->SetThermalSchedule(fSolidMatSupport.Schedule(LTfnum));
			
			/* set flag */
			fHasThermal = true;
		}
		
		/* perform initialization */
		pmat->Initialize();
	}  } /* end try */
	
	catch (ExceptionT::CodeT error)
	{
		cout << "\n SolidMatList1DT::ReadMaterialData: exception constructing material " << i+1
		     << '\n' << "     index " << matnum+1 << ", code " << matcode << endl;
		throw error;
	}
}


/* error messages */

void SolidMatList1DT::Error_no_small_strain(ostream& out, int matcode) const
{
	out << "\n SolidMatList1DT: material " << matcode
		<< " requires a small strain element" << endl;
	throw ExceptionT::kBadInputValue;
}

void SolidMatList1DT::Error_no_finite_strain(ostream& out, int matcode) const
{
	out << "\n SolidMatList1DT: material " << matcode
		<< " requires a finite strain element" << endl;
	throw ExceptionT::kBadInputValue;
}

void SolidMatList1DT::Error_no_multi_scale(ostream& out, int matcode) const
{
	out << "\n SolidMatList1DT: material " << matcode
		<< " requires a variational multi-scale element" << endl;
	throw ExceptionT::kBadInputValue;
}

