/* $Id: MRSSNLHardT.h,v 1.15 2010-07-22 16:55:35 regueiro Exp $ */
/* created: Majid T. Manzari */
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
	MRSSNLHardT(int num_ip, double mu, double lambda);

	/* status flags */
	enum LoadingStatusT {kIsPlastic = 0,
                         kIsElastic = 1,
						 kIsLocalized = 2,                         
                         kReset = 3}; // indicate not to repeat update
                         
    enum InternalVariablesT {kchi = 0,  // stress-like internal state variable
	                           kc = 1,
	                      ktanphi = 2,
	                      ktanpsi = 3,
	                      kftrial = 4, // trial yield function value
	                          kff = 5, // current yield function value
                         kdlambda = 6, // consistency parameter
                      kstressnorm = 7, // norm of residuals 
                       kindicator = 8}; // indicator                     

	/* returns elastic strain (3D) */
	virtual const dSymMatrixT& ElasticStrain(const dSymMatrixT& totalstrain, 
		const ElementCardT& element, int ip);
		                             
	/* return correction to stress vector computed by mapping the
	 * stress back to the yield surface, if needed */
	const dSymMatrixT& StressCorrection(const dSymMatrixT& totalstrain, 
		ElementCardT& element, int ip, int iter); 

    void dfdSig_f(const dSymMatrixT& Sig, const dArrayT& qn, dArrayT& dfdSig);    
    void dfdq_f(const dSymMatrixT& Sig, const dArrayT& qn, dArrayT& dfdq); 
    void dQdSig_f(const dSymMatrixT& Sig, const dArrayT& qn, dArrayT& dQdSig);   
    void dQdSig2_f(const dSymMatrixT& Sig, const dArrayT& qn, dMatrixT& dQdSig2);
    void dQdSigdq_f(const dSymMatrixT& Sig, const dArrayT& qn, dMatrixT& dQdSigdq);
    void qbar_f(const dSymMatrixT& Sig, const dArrayT& qn, dArrayT& qbar);
    void dqbardSig_f(const dSymMatrixT& Sig, const dArrayT& qn, dMatrixT& dqbardSig);
    void dqbardq_f(const dSymMatrixT& Sig, const dArrayT& qn, dMatrixT& dqbardq);
	
    /* utility function */
	double signof(double r);

    /* double contraction of fourth order tensor with second
	 * order tensor    */
	void Contract4To2(const dMatrixT& mat,const dSymMatrixT& vec, 
	                  dSymMatrixT& res);
	                  
	/* return the correction to moduli due to plasticity (if any)
	 *
	 * Note: Return mapping occurs during the call to StressCorrection.
	 *       The element passed in is already assumed to carry current
	 *       internal variable values */
	const dMatrixT& Moduli(const ElementCardT& element, int ip); 

	/* Modulus for checking perfectly plastic bifurcation */
	const dMatrixT& ModuliPerfPlas(const ElementCardT& element, int ip);

	/* return a pointer to a new plastic element object constructed with
	 * the data from element */
	void AllocateElement(ElementCardT& element);

	/** internal variables */
	dArrayT& Internal(void) { return fInternal; };
	
	/* element level data */
	void Update(ElementCardT& element);
	void Reset(ElementCardT& element);

	/* returns 1 if the trial elastic strain state lies outside of the 
	 * yield surface */
	int PlasticLoading(const dSymMatrixT& totalstrain, ElementCardT& element, int ip);

	/* computes the deviatoric stress corresponding to the given element
	 * and elastic strain.  The function returns a reference to the
	 * stress in fDevStress */
	dSymMatrixT& DeviatoricStress(const dSymMatrixT& trialstrain, 
		const ElementCardT& element);

	/* computes the hydrostatic (mean) stress. */
	double MeanStress(const dSymMatrixT& trialstrain,
		const ElementCardT& element);

	/** \name implementation of the ParameterInterfaceT interface */
	/*@{*/
	/** accept parameter list */
	virtual void TakeParameterList(const ParameterListT& list);
	/*@}*/
	
  private:

	/* load element data for the specified integration point */
	void LoadData(const ElementCardT& element, int ip);

  protected:

  	/* element level internal state variables */
  	dSymMatrixT fPlasticStrain; //total plastic strain (deviatoric and volumetric)
  	dSymMatrixT fStress;        // updated Cauchy stress
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
    dMatrixT    fModuliPerfPlas;
  		
	/* work space */
	dSymMatrixT fDevStress;
	dSymMatrixT fDevStrain; /* deviatoric part of the strain tensor */
};

} // namespace Tahoe 
#endif /* _MR_SS_NL_HARD_T_H_ */
