/* $Id: DPSSLinHardT.h,v 1.9 2001-09-20 23:46:00 cfoster Exp $ */
/* created: myip (06/01/1999)                                      */
/*  
 * Interface for Drucker-Prager, nonassociative, small strain,
 * pressure-dependent plasticity model with linear isotropic hardening.
 *
 *	Note: all calculations are peformed in 3D.
 */

#ifndef _DP_SS_LIN_HARD_T_H_
#define _DP_SS_LIN_HARD_T_H_

/* base class */
#include "DPPrimitiveT.h"

/* direct members */
#include "dSymMatrixT.h"
#include "dMatrixT.h"
#include "dArrayT.h"

/* forward declarations */
class ElementCardT;

class DPSSLinHardT: public DPPrimitiveT
{
  public:

	/* constructor */
	DPSSLinHardT(ifstreamT& in, int num_ip, double mu, double lambda);

  	/* output name */
	virtual void PrintName(ostream& out) const;

  protected:

	/* status flags */
	enum LoadingStatusT {kIsPlastic = 0,
                             kIsElastic = 1,
                             kReset = 3}; // indicate not to repeat update

	/* returns elastic strain (3D) */
	virtual const dSymMatrixT& ElasticStrain(const dSymMatrixT& totalstrain, 
		const ElementCardT& element, int ip);
			
	/* return correction to stress vector computed by mapping the
	 * stress back to the yield surface, if needed */
	const dSymMatrixT& StressCorrection(const dSymMatrixT& trialstrain, 
		ElementCardT& element, int ip); 

	/* return the correction to moduli due to plasticity (if any)
	 *
	 * Note: Return mapping occurs during the call to StressCorrection.
	 *       The element passed in is already assumed to carry current
	 *       internal variable values */
	const dMatrixT& ModuliCorrection(const ElementCardT& element, int ip); 

        /* Modulus for checking discontinuous bifurcation */

	const dMatrixT& ModuliCorrDisc(const ElementCardT& element, int ip);

	/* return a pointer to a new plastic element object constructed with
	 * the data from element */
	void AllocateElement(ElementCardT& element);

	enum InternalVariablesT {kalpha = 0,  // stress-like internal state variable
                        kstressnorm = 1,  // norm of stress
                            kdgamma = 2,  // consistency parameter
                            kftrial = 3, // yield function value
			    kdgamma2 = 4}; // 2nd consistency par. at vertex
	/* element level data */
	void Update(ElementCardT& element);
	void Reset(ElementCardT& element);

	/* returns 1 if the trial elastic strain state lies outside of the 
	 * yield surface */
	int PlasticLoading(const dSymMatrixT& trialstrain, 
                           const ElementCardT& element, 
		           int ip);

	/* computes the deviatoric stress corresponding to the given element
	 * and elastic strain.  The function returns a reference to the
	 * stress in fDevStress */
	dSymMatrixT& DeviatoricStress(const dSymMatrixT& trialstrain, 
		const ElementCardT& element);

	/* computes the hydrostatic (mean) stress. */
	double MeanStress(const dSymMatrixT& trialstrain,
		const ElementCardT& element);

  private:

	/* load element data for the specified integration point */
	void LoadData(const ElementCardT& element, int ip);

	/* returns 1 if the trial elastic strain state lies outside of the 
	 * yield surface */
	//	int PlasticLoading(const dSymMatrixT& trialstrain, 
        //                         ElementCardT& element, 
	//	                   int ip);

	/* computes the deviatoric stress corresponding to the given element
	 * and elastic strain.  The functions returns a reference to the
	 * stress in fDevStress */
	//	dSymMatrixT& DeviatoricStress(const dSymMatrixT& totalstrain, 
	//		const ElementCardT& element);

	/* computes the hydrostatic (mean) stress. */  
	//	double& MeanStress(const dSymMatrixT& totalstrain,
	//		const ElementCardT& element);

  protected:

  	/* element level internal state variables */
  	dSymMatrixT fPlasticStrain; //total plastic strain (deviatoric and volumetric)
  	dSymMatrixT fUnitNorm;      //unit normal to the yield surface
  	dArrayT     fInternal;      //internal variables

  private:

	/* number of integration points */
	int fNumIP;

  	/* material parameters **/
  	double fmu;
	double flambda;
	double fkappa;
	double fX_H;
        double fX;
        double fMeanStress;
  
  	/* return values */
  	dSymMatrixT	fElasticStrain;
  	dSymMatrixT	fStressCorr;
  	dMatrixT	fModuliCorr;
        dMatrixT        fModuliCorrDisc;
  		
	/* work space */
	dSymMatrixT fDevStress;
	dSymMatrixT fDevStrain; /* deviatoric part of the strain tensor */
	dSymMatrixT IdentityTensor2;  

	dMatrixT      fTensorTemp;
	dSymMatrixT   One;  
  	
};

#endif /* _DP_SS_LIN_HARD_T_H_ */