/* $Id: FDSimoViscoBaseT.h,v 1.2 2003-04-05 20:38:07 thao Exp $ */
/* created:   TDN (5/31/2001) */

#ifndef _FD_SIMO_VISCO_BASE_H_
#define _FD_SIMO_VISCO_BASE_H_
 
#include "FSSolidMatT.h"
#include "dSymMatrixT.h"

namespace Tahoe {

/* forward declarations */
class ifstreamT;

/*small strain linear viscoelastic constitutive law */
class FDSimoViscoBaseT: public FSSolidMatT
{
	public:

	/*constructor*/
	FDSimoViscoBaseT(ifstreamT& in, const FSMatSupportT& support);

	/** return the pressure associated with the last call to 
	 * SolidMaterialT::s_ij. \note NOT IMPLEMENTED */
	virtual double Pressure(void) const {
		cout << "\n FDSimoViscoBaseT::Pressure: not implemented" << endl;
		throw ExceptionT::kGeneralFail;
		return 0.0;
	};

	/*print parameters*/
	void Print(ostream& out) const;
	void PrintName(ostream& out) const;
		
	/* apply pre-conditions at the current time step */
	void InitStep(void){FSSolidMatT::InitStep();};

	/*initialize history variable*/
	bool NeedsPointInitialization(void) const {return true;}; // declare true
	void PointInitialize(void);                // assigns storage space
	
	/* update/reset internal variables */
	void UpdateHistory(void); // element at a time
	void ResetHistory(void);  // element at a time
	void Load(ElementCardT& element, int ip);
	void Store(ElementCardT& element, int ip);

	//compute output variables
	virtual int NumOutputVariables() const;
	virtual void OutputLabels(ArrayT<StringT>& labels) const;
	virtual void ComputeOutput(dArrayT& output);

	virtual double StrainEnergyDensity(void);

	protected:

	enum Spring {kEquilibrium = 0, kNonEquilibrium = 1};
        protected:

	/*number of state variables*/
	int fnstatev;
	
	/* Internal state variables array*/
	dArrayT fstatev;

	/*preceding values*/		 
	/*deviatoric*/
	dSymMatrixT   fdevQ_n;
	dSymMatrixT   fdevSin_n;

	/*mean*/
	dArrayT        fmeanQ_n;
	dArrayT        fmeanSin_n;
	
	/*current values*/
	/*deviatoric*/
	dSymMatrixT   fdevQ;
	dSymMatrixT   fdevSin;
	/*mean*/
	dArrayT   fmeanQ;
	dArrayT   fmeanSin;

};
}
#endif /*_FD_SIMO_VISCO_BASE_H_*/
