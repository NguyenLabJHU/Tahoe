/* $Id: RGBaseT.h,v 1.2.2.1 2002-10-17 04:38:00 paklein Exp $ */
/* created : TDN (1/22/2001) */

#ifndef _RG_BASE_T_H_
#define _RG_BASE_T_H_

/* base classes */
#include "FDStructMatT.h"
#include "IsotropicT.h"

/* direct members */
#include "SpectralDecompT.h"


namespace Tahoe {

/** base class for large deformation isotropic material following
 * Ogden's formulation */
class RGBaseT: public FDStructMatT, public IsotropicT
{
  public:
  
	/* constructor */
	RGBaseT(ifstreamT& in, const FiniteStrainT& element);

	/** return the pressure associated with the last call to 
	 * StructuralMaterialT::s_ij. \note NOT IMPLEMENTED */
	virtual double Pressure(void) const {
		cout << "\n RGBaseT::Pressure: not implemented" << endl;
		throw ExceptionT::kGeneralFail;
		return 0.0;
	};

	/* print parameters */	
	virtual	void Print(ostream& out) const;	
	virtual void PrintName(ostream& out) const;
	
	/*Initialize history variable*/
	virtual bool NeedsPointInitialization(void) const {return true;}; 
	virtual void PointInitialize(void);              

	/* update/reset internal variables */
	virtual void UpdateHistory(void); // element at a time
	virtual void ResetHistory(void);  // element at a time
	/* apply pre-conditions at the current time step */
	virtual void InitStep(void){ FDStructMatT::InitStep(); };
	
	/* form of tangent matrix (symmetric by default) */
	virtual GlobalT::SystemTypeT TangentType(void) const;

	void Load(ElementCardT& element, int ip);
	void Store(ElementCardT& element, int ip);

	/* return true of model is purely 2D, plain stress */
	virtual bool PurePlaneStress(void) const { return false; };

  protected:
  
	/* construct symmetric rank-4 mixed-direction tensor (6.1.44) */
  	void MixedRank4_2D(const dArrayT& a, const dArrayT& b, 
  		dMatrixT& rank4_ab) const;
  	void MixedRank4_3D(const dArrayT& a, const dArrayT& b, 
  		dMatrixT& rank4_ab) const;

  protected:

	/* spectral operations */
	SpectralDecompT fSpectralDecompSpat;
	SpectralDecompT fSpectralDecompRef;
	const GlobalT::StateT& fRunState;

	/*Internal state variables*/
	dSymMatrixT     fC_v;
	dSymMatrixT     fC_vn;
	
	/*number of state variables*/
	int fnstatev;
	
	/* internal state variables array*/
	dArrayT fstatev;

  	/*Time increment*/
  	const double& fdt;
};
}

#endif /* _RG_Base_T_H_ */

