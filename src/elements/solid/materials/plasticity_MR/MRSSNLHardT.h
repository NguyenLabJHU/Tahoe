/* created: Majid T. Manzari (04/16/2003)            */
/*  
 * Interface for a nonassociative, small strain,     */
/* pressure dependent plasticity model with nonlinear 
/* isotropic hardening/softening.                    */

/*
 *	Note: all calculations are peformed in 3D.
 */

#ifndef _MR_SS_LIN_HARD_T_H_
#define _MR_SS_LIN_HARD_T_H_

/* base class */
#include "MRPrimitiveT.h"

/* direct members */
#include "dSymMatrixT.h"
#include "dMatrixT.h"
#include "dArrayT.h"

namespace Tahoe {

/* forward declarations */
class ElementCardT;

class MRSSNLHardT: public MRPrimitiveT
{

public:
	/* constructor */
	MRSSNLHardT(ifstreamT& in, int num_ip, double mu, double lambda);

  	/* output name */
	virtual void PrintName(ostream& out) const;

  protected:

	/* status flags */
	enum LoadingStatusT {kIsPlastic = 0,
                         kIsElastic = 1,
                             kReset = 3}; // indicate not to repeat update
                             
	/* return correction to stress vector computed by mapping the
	 * stress back to the yield surface, if needed */
	const dSymMatrixT& StressCorrection(const dSymMatrixT& trialstrain, 
		ElementCardT& element, int ip); 
		
	virtual const dSymMatrixT& ElasticStrain(const dSymMatrixT& totalstrain, 
	                                 const ElementCardT& element, int ip);
		
	double& Yield_f(const dArrayT& Sig, const dArrayT& qn, double& ff);
    dArrayT& qbar_f(const dArrayT& Sig, const dArrayT& qn, dArrayT& qbar);
    dArrayT& dfdSig_f(const dArrayT& Sig, const dArrayT& qn, dArrayT& dfdSig);
    dArrayT& dQdSig_f(const dArrayT& Sig, const dArrayT& qn, dArrayT& dQdSig);    
    dArrayT& dfdq_f(const dArrayT& Sig, const dArrayT& qn, dArrayT& dfdq);    
    dMatrixT& dQdSig2_f(const dArrayT& qn, dMatrixT& dQdSig2);
    dMatrixT& dQdSigdq_f(const dArrayT& Sig, const dArrayT& qn, dMatrixT& dQdSigdq);
    dMatrixT& dqbardSig_f(const dArrayT& Sig, const dArrayT& qn, dMatrixT& dqbardSig);
    dMatrixT& dqbardq_f(const dArrayT& Sig, const dArrayT& qn, dMatrixT& dqbardq);
    
    /* utility function */
	double signof(double& r);

	/* return the correction to moduli due to plasticity (if any)
	 *
	 * Note: Return mapping occurs during the call to StressCorrection.
	 *       The element passed in is already assumed to carry current
	 *       internal variable values */
	const dMatrixT& Moduli(const ElementCardT& element, int ip); 

        /* Modulus for checking discontinuous bifurcation */

	const dMatrixT& ModuliDisc(const ElementCardT& element, int ip);

	/* return a pointer to a new plastic element object constructed with
	 * the data from element */
	void AllocateElement(ElementCardT& element);

	enum InternalVariablesT {kchi = 18,  // stress-like internal state variable
	                         kc   = 19,
	                      ktanphi = 20,
	                      ktanpsi = 21,
                      kstressnorm = 22,  // norm of residuals
                         kdlambda = 23,  // consistency parameter
                         kplastic = 24,  // Plastic Index
                          kftrial = 27}; // yield function value

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

  protected:

  	/* element level internal state variables */
  	dSymMatrixT fPlasticStrain; //total plastic strain (deviatoric and volumetric)
  	dArrayT     fInternal;      //internal variables

  private:

	/* number of integration points */
	int fNumIP;

  	/* material parameters **/
  	double fmu;
	double flambda;
	double fkappa;
    double fMeanStress;
  
  	/* return values */
  	dSymMatrixT	fElasticStrain;
  	dSymMatrixT	fStressCorr;
  	dMatrixT	fModuli;
    dMatrixT    fModuliDisc;
  		
	/* work space */
	dSymMatrixT fDevStress;
	dSymMatrixT fDevStrain; /* deviatoric part of the strain tensor */
	dSymMatrixT IdentityTensor2;  

	dMatrixT      fTensorTemp;
	dSymMatrixT   One;  
  	
};

} // namespace Tahoe 
#endif /* _DMR_SS_NL_HARD_T_H_ */
