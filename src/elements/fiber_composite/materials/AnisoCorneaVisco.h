/* $Id: AnisoCorneaVisco.h,v 1.1 2006-08-10 01:35:44 thao Exp $ */
/* created: TDN (01/22/2001) */
#ifndef _AnisoCorneaVisco_
#define _AnisoCorneaVisco_ 
 
/* base class */
#include "FSFiberMatViscT.h"


namespace Tahoe {
/*forward declarations*/
class CirclePointsT;
class C1FunctionT;

class AnisoCorneaVisco: public  FSFiberMatViscT
{
   public:
  
	/* constructor/destructor */
	AnisoCorneaVisco(void);
	
/* destructor */
	~AnisoCorneaVisco(void);
	
	/** required parameter flag. Indicates whether the constitutive model
	 * requires the deformation gradient from the previous time increment.
	 * \return false by default. */
	virtual bool Need_F_last(void) const { return true; };

	/* strain energy density */
	virtual double StrainEnergyDensity(void);

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** describe the parameters needed by the interface */
	virtual void DefineParameters(ParameterListT& list) const;

	/** information about subordinate parameter lists */
	virtual void DefineSubs(SubListT& sub_list) const;

	/** a pointer to the ParameterInterfaceT of the given subordinate */
	virtual ParameterInterfaceT* NewSub(const StringT& name) const;

	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/

protected:
	/*calculates  matrix contribution to 2PK stress*/
	virtual void ComputeMatrixStress (const dSymMatrixT& Stretch, const dSymMatrixT& Stretch_v, 
				dSymMatrixT& Stress, const int process_index, const int fillmode = dSymMatrixT::kOverwrite);

	/*computes matrix moduli*/
	virtual void ComputeMatrixMod (const dSymMatrixT& Stretch, const dSymMatrixT& Stretch_v, dSymMatrixT& Stress,
				dMatrixT& Mod, const int process_index, const int fillmode = dSymMatrixT::kOverwrite);

	/*computes fiber stress in local frame*/
	virtual void ComputeFiberStress (const dSymMatrixT& Stretch, const dSymMatrixT& Stretch_v, dSymMatrixT& Stress, 
				const int process_index);
	
	/*computes  moduli in local frame*/
	virtual void ComputeFiberMod (const dSymMatrixT& Stretch, const dSymMatrixT& Stretch_v, dSymMatrixT& Stress, dSymMatrixT& Mod, 
				const int process_index);


	/*compute the algorithmic moduli dSNEQ/dCv deltaCv/deltadC in local fiber coord sys*/
	virtual void ComputeCalg (const dSymMatrixT& Stretch, const dSymMatrixT& Stretch_v,  dMatrixT& Calg, const int process_index); 
	
	/*local newton loop for viscous stretch tensor*/ 
	virtual void Compute_Cv(const dSymMatrixT& C_last, const dSymMatrixT& C, const dSymMatrixT& Cv_last, dSymMatrixT& Cv, const int process_index);


	/*computes flow stress in local frame*/
	void ComputeFlowStress (const dSymMatrixT& Stretch, const dSymMatrixT& Stretch_v, dSymMatrixT& FlowStress, 
				const int process_index);

	/*computes dFlowStress/dC in local frame.  Note for this model, dFlowStress/dC = - dSNEQ/dCv*/
	void dFlowdC (const dSymMatrixT& FiberStretch, const dSymMatrixT& FiberStretch_v, dSymMatrixT& FiberMod,  const int pindex);

	/*computes dFlowStress/dCv in local frame*/
	void dFlowdCv (const dSymMatrixT& FiberStretch, const dSymMatrixT& FiberStretch_v, dSymMatrixT& FiberMod,  const int pindex);

	/*returns viscosity tensor for given C and Cv in local frame*/
	void ComputeViscosity(const dSymMatrixT& Stretch, const dSymMatrixT& Stretch_v,  dSymMatrixT& Visc, 
				const int process_index);

	/*returns viscosity tensor for given C and Cv in local frame, dV^-1_IK/dCv_J Sig_K/*/
	void ComputeDViscDCv(const dSymMatrixT& Stretch, const dSymMatrixT& Stretch_v, const dArrayT& Vec, dSymMatrixT& DVisc, 
				const int process_index);

	void ComputeDViscDC(const dSymMatrixT& Stretch, const dSymMatrixT& Stretch_v,  const dArrayT& Vec, dSymMatrixT& DVisc, 
				const int process_index);
				
	/* strained lengths in terms of the Lagrangian stretch eigenvalues */
	void CompI4(const dSymMatrixT& stretch);
	void CompI4(const dSymMatrixT& stretch, const dSymMatrixT& stretch_v);
	
private:

	/* initialize angle tables */
	void Construct(void);

protected:
	
	/*constitutive values for matrix*/
	double fMu;
	double fGamma;
	
	/* integration point generator */
	CirclePointsT*	fCircle;

	/* potential function */
	/*dimension fNumProcess + 1*/
	ArrayT<C1FunctionT*> fPotential;

	/* fibril distribution function */
	C1FunctionT* fDistribution;

	/* inverse viscosity function */
	/*dimension fNumProcess*/
	ArrayT<C1FunctionT*> fViscosity;
	/*invserse of the viscosity*/
	dSymMatrixT fiVisc;
	
	/*workspaces*/
	/*workspaces*/
	dSymMatrixT fFlowStress;
	dSymMatrixT fResidual;
	dMatrixT fiK;
	dMatrixT fG;
	dSymMatrixT fMod1;
	dMatrixT fMod2;
	dArrayT fVec;

	/* length table */
	/*I4 */
	dArrayT	fI4; /*C:M*/
	dArrayT	fI4v; /*Cv:M*/
	
	/* potential tables */
	dArrayT	fU;
	dArrayT	fdU;
	dArrayT	fddU;

	/* jacobian table */
	dArrayT	fjacobian;

	/* STRESS angle tables for fiber stress - by associated stress component */
	dArray2DT fStressTable;
	  	
	/* MODULI angle tables for fiber moduli */
	dArray2DT fModuliTable;	
};
	
}
#endif /* _AnisoCorneaVisco_ */
