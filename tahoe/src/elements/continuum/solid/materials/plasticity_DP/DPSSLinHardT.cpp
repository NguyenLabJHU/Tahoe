/* $Id: DPSSLinHardT.cpp,v 1.8 2001-07-27 00:08:04 cfoster Exp $ */
/* created: myip (06/01/1999)                                        */
/*
 * Interface for Drucker-Prager, nonassociative, small strain,
 * pressure-dependent plasticity model with linear isotropic hardening
 *
 */

#include "DPSSLinHardT.h"
#include <iostream.h>
#include <math.h>

#include "iArrayT.h"
#include "ElementCardT.h"
#include "StringT.h"

/* class constants */
const int    kNumInternal = 4; // number of internal state variables
const double sqrt23       = sqrt(2.0/3.0);
const double sqrt32       = sqrt(3.0/2.0);
const double kYieldTol    = 1.0e-10;
const int    kNSD         = 3;

/* constructor */
DPSSLinHardT::DPSSLinHardT(ifstreamT& in, int num_ip, double mu, double lambda):
	DPPrimitiveT(in),
	fNumIP(num_ip),
	fmu(mu),
	flambda(lambda),
	fkappa(flambda + (2.0/3.0*fmu)),
	fX_H(3.0*(fmu+ffriction*fdilation*fkappa) + fH_prime),
        fX(3.0*(fmu+ffriction*fdilation*fkappa)), //for discontinuous bifurcation check
	fElasticStrain(kNSD),
	fStressCorr(kNSD),
	fModuliCorr(dSymMatrixT::NumValues(kNSD)),
	   fModuliCorrDisc(dSymMatrixT::NumValues(kNSD)), //for disc check
	fDevStress(kNSD),
	fMeanStress(0.0),
	fDevStrain(kNSD), 
	fTensorTemp(dSymMatrixT::NumValues(kNSD)),
        IdentityTensor2(kNSD),
	One(kNSD)
{
  /* initialize constant tensor */
  One.Identity();
}

/* returns elastic strain */
const dSymMatrixT& DPSSLinHardT::ElasticStrain(const dSymMatrixT& totalstrain, 
	const ElementCardT& element, int ip)
{
	/* remove plastic strain */
	if (element.IsAllocated()) 
	{
		/* load internal variables */
		LoadData(element, ip);

		/* compute elastic strain */
		fElasticStrain.DiffOf(totalstrain, fPlasticStrain);
	
		return fElasticStrain;
	}	
	/* no plastic strain */
	else	
		return totalstrain;
}

/* return correction to stress vector computed by mapping the
 * stress back to the yield surface, if needed */
const dSymMatrixT& DPSSLinHardT::StressCorrection(
        const dSymMatrixT& trialstrain, 
	ElementCardT& element, int ip)
{
	/* check consistency and initialize plastic element */
	if (PlasticLoading(trialstrain, element, ip) && 
	    !element.IsAllocated())
	{
		/* new element */
		AllocateElement(element);
					
		/* initialize element data */
		PlasticLoading(trialstrain, element, ip);
	}

	/* initialize */
	fStressCorr = 0.0;
	
	if (element.IsAllocated()) 
	{		
		/* fetch data */
		double  ftrial = fInternal[kftrial];
		double& dgamma = fInternal[kdgamma];

		/* return mapping (single step) */
		if (ftrial > kYieldTol)
		{
		/* plastic increment */
	        dgamma = ftrial/fX_H;

		/* plastic increment stress correction */
		fStressCorr.PlusIdentity(-sqrt(3.0)*fdilation*fkappa*dgamma);
		fStressCorr.AddScaled(-sqrt(6.0)*fmu*dgamma, fUnitNorm);

		//construct the trial stress
		dSymMatrixT stress(DeviatoricStress(trialstrain, element));
		stress.PlusIdentity(MeanStress(trialstrain,element));
		
		//corrected stress and internal state variables
		stress += fStressCorr;
		double a = fInternal[kalpha] - fH_prime*dgamma;

		// evaluate plastic consistency
		double p = stress.Trace()/3.0;
		//cout << " pressure  = " << p << endl;
		
		double f = YieldCondition(stress.Deviatoric(), p, a);
		//cout << " yield function = " << f << endl;
		}
		else
		dgamma = 0.0;			
	}
		
	return fStressCorr;
}	

/* return the correction to moduli due to plasticity (if any)
 *
 * Note: Return mapping occurs during the call to StressCorrection.
 *       The element passed in is already assumed to carry current
 *       internal variable values */
const dMatrixT& DPSSLinHardT::ModuliCorrection(const ElementCardT& element, 
	int ip)
{
	/* initialize */
	fModuliCorr = 0.0;

	cout << "element.IsAllocated = " << element.IsAllocated() << '\n';
	cout << "element.IntegerData()  = " << element.IntegerData() << '\n'; 

	if (element.IsAllocated() && 
	   (element.IntegerData())[ip] == kIsPlastic)
	{
        
	  cout << " in if statement \n";

		/* load internal state variables */
	  	LoadData(element,ip);
		
	double c1  = -3.0*ffriction*fdilation*fkappa*fkappa/fX_H;
	       c1 += (4.0/3.0)*sqrt32*fmu*fmu*fInternal[kdgamma]/fInternal[kstressnorm];
	double c2  = -sqrt(6.0)*fmu*fmu*fInternal[kdgamma]/fInternal[kstressnorm];
	double c3  = -(3.0/2.0)/fX_H + sqrt32*fInternal[kdgamma]/fInternal[kstressnorm];
	       c3 *= 4.0*fmu*fmu;

	double c4  = -3.0*sqrt(2.0)*fkappa*fmu/fX_H;

		fTensorTemp.Outer(One, One);
		fModuliCorr.AddScaled(c1, fTensorTemp);

 	    	fTensorTemp.ReducedIndexI();
		fModuliCorr.AddScaled(2.0*c2, fTensorTemp);

		fTensorTemp.Outer(fUnitNorm,fUnitNorm);
		fModuliCorr.AddScaled(c3, fTensorTemp);

		fTensorTemp.Outer(One,fUnitNorm);
		fModuliCorr.AddScaled(fdilation*c4, fTensorTemp);

		fTensorTemp.Outer(fUnitNorm,One);
		fModuliCorr.AddScaled(ffriction*c4, fTensorTemp);
	}

               cout << " Moduli Correction = \n";
	       //        cout << fModuliCorr[0] << ' ' <<  fModuliCorr[6] << ' ' << fModuliCorr[12] << fModuliCorr[18] << ' ' <<  fModuliCorr[24] << ' ' << fModuliCorr[30] << '\n';
	       // cout << fModuliCorr[1] << ' ' <<  fModuliCorr[7] << ' ' << fModuliCorr[13] << fModuliCorr[19] << ' ' <<  fModuliCorr[25] << ' ' << fModuliCorr[31] << '\n';
	       // cout << fModuliCorr[2] << ' ' <<  fModuliCorr[8] << ' ' << fModuliCorr[14] << fModuliCorr[20] << ' ' <<  fModuliCorr[26] << ' ' << fModuliCorr[32] << '\n'; 
	       // cout << fModuliCorr[3] << ' ' <<  fModuliCorr[9] << ' ' << fModuliCorr[15] << fModuliCorr[21] << ' ' <<  fModuliCorr[27] << ' ' << fModuliCorr[33] << '\n';
	       // cout << fModuliCorr[4] << ' ' <<  fModuliCorr[10] << ' ' << fModuliCorr[16] << fModuliCorr[22] << ' ' <<  fModuliCorr[28] << ' ' << fModuliCorr[34] << '\n';
	       // cout << fModuliCorr[5] << ' ' <<  fModuliCorr[11] << ' ' << fModuliCorr[17] << fModuliCorr[23] << ' ' <<  fModuliCorr[29] << ' ' << fModuliCorr[35] << '\n';
	
               cout << fModuliCorr << '\n';
	

	return fModuliCorr;
}	


/* return the correction to modulus Cep~, checking for discontinuous
 *   bifurcation */


const dMatrixT& DPSSLinHardT::ModuliCorrDisc(const ElementCardT& element, 
	int ip)
{
	/* initialize */
	fModuliCorrDisc = 0.0;

	if (element.IsAllocated() && 
	   (element.IntegerData())[ip] == kIsPlastic)
	{
		/* load internal state variables */
	  	LoadData(element,ip);
		
	double c1d  = -3.0*ffriction*fdilation*fkappa*fkappa/fX;
	       c1d += (4.0/3.0)*sqrt32*fmu*fmu*fInternal[kdgamma]/fInternal[kstressnorm];
	double c2d  = -sqrt(6.0)*fmu*fmu*fInternal[kdgamma]/fInternal[kstressnorm];
	double c3d  = -(3.0/2.0)/fX + sqrt32*fInternal[kdgamma]/fInternal[kstressnorm];
	       c3d *= 4.0*fmu*fmu;
	double c4d  = -3.0*sqrt(2.0)*fkappa*fmu/fX; 

		fTensorTemp.Outer(One, One);
		fModuliCorrDisc.AddScaled(c1d, fTensorTemp);

 	    	fTensorTemp.ReducedIndexI();
		fModuliCorrDisc.AddScaled(2.0*c2d, fTensorTemp);

		fTensorTemp.Outer(fUnitNorm,fUnitNorm);
		fModuliCorrDisc.AddScaled(c3d, fTensorTemp);

		fTensorTemp.Outer(One,fUnitNorm);
		fModuliCorrDisc.AddScaled(fdilation*c4d, fTensorTemp);

		fTensorTemp.Outer(fUnitNorm,One);
		fModuliCorrDisc.AddScaled(ffriction*c4d, fTensorTemp);
	}
	return fModuliCorrDisc;
}	

 	 	
/* return a pointer to a new plastic element object constructed with
 * the data from element */
void DPSSLinHardT::AllocateElement(ElementCardT& element)
{
	/* determine storage */
	int i_size = 0;
	i_size += fNumIP; //fFlags

	int d_size = 0;
	d_size += dSymMatrixT::NumValues(kNSD)*fNumIP; //fPlasticStrain
	d_size += dSymMatrixT::NumValues(kNSD)*fNumIP; //fUnitNorm
	//	d_size += NumValues(kNSD)*fNumIP; //fBeta
	d_size += kNumInternal*fNumIP;        //fInternal

	/* construct new plastic element */
	element.Allocate(i_size, d_size);
	
	/* initialize values */
	element.IntegerData() = kIsElastic;
	element.DoubleData()  = 0.0;  // initialize all double types to 0.0
}

/***********************************************************************
 * Protected
 ***********************************************************************/

void DPSSLinHardT::PrintName(ostream& out) const
{
	/* inherited */
	DPPrimitiveT::PrintName(out);

	out << "    Small Strain\n";
}

/* element level data */
void DPSSLinHardT::Update(ElementCardT& element)
{
	/* get flags */
	iArrayT& Flags = element.IntegerData();

	/* check if reset state */
	if (Flags[0] == kReset)
	{
		Flags = kIsElastic; //don't update again
		return; 
	}

	/* update plastic variables */
	for (int ip = 0; ip < fNumIP; ip++)
		if (Flags[ip] == kIsPlastic) /* plastic update */
		{
			/* do not repeat if called again. */
			Flags[ip] = kIsElastic;
			/* NOTE: ComputeOutput writes the updated internal variables
			 *       for output even during iteration output, which is
			 *       called before UpdateHistory */

			/* fetch element data */
			LoadData(element, ip);
	
			/* plastic increment */
			double& dgamma = fInternal[kdgamma];
			//cout << fInternal[kdgamma] << endl;
		
			/* internal state variable */
			fInternal[kalpha] -= fH_prime*dgamma;

			/* dev plastic strain increment	*/
			fPlasticStrain.AddScaled( sqrt32*dgamma, fUnitNorm );
        	
        	/* vol plastic strain increment	*/
			fPlasticStrain.AddScaled( fdilation*dgamma/sqrt(3.0), One );
		}
}

/* resets to the last converged solution */
void DPSSLinHardT::Reset(ElementCardT& element)
{
	/* flag not to update again */
	(element.IntegerData()) = kReset;
}

/***********************************************************************
 * Private
 ***********************************************************************/

/* load element data for the specified integration point */
void DPSSLinHardT::LoadData(const ElementCardT& element, int ip)
{
	/* check */
	if (!element.IsAllocated()) throw eGeneralFail;

	/* fetch arrays */
	dArrayT& d_array = element.DoubleData();
	
	/* decode */
	int stressdim = dSymMatrixT::NumValues(kNSD);
	int offset    = stressdim*fNumIP;
	int dex       = ip*stressdim;
	
	fPlasticStrain.Set(        kNSD, &d_array[           dex]);
	     fUnitNorm.Set(        kNSD, &d_array[  offset + dex]);     
	     fInternal.Set(kNumInternal, &d_array[2*offset + ip*kNumInternal]);
}

/* returns 1 if the trial elastic strain state lies outside of the 
 * yield surface */
int DPSSLinHardT::PlasticLoading(const dSymMatrixT& trialstrain, 
	const ElementCardT& element, int ip)
{
	/* not yet plastic */
	if (!element.IsAllocated()) 
		return( YieldCondition(DeviatoricStress(trialstrain,element),
			       MeanStress(trialstrain,element), 0.0) > kYieldTol );
        /* already plastic */
	else 
	{
	/* get flags */
	iArrayT& Flags = element.IntegerData();
		
	/* load internal variables */
	LoadData(element, ip);
		
	fInternal[kftrial] = YieldCondition(DeviatoricStress(trialstrain,element),
	    MeanStress(trialstrain,element),fInternal[kalpha]);

		/* plastic */
		if (fInternal[kftrial] > kYieldTol)
		{		
			/* compute unit normal */
			double& norm = fInternal[kstressnorm];

			norm = sqrt(fDevStress.ScalarProduct());
			fUnitNorm.SetToScaled(1.0/norm, fDevStress);
		
			/* set flag */
			Flags[ip] = kIsPlastic;
	
			return 1;
		}
		/* elastic */
		else
		{
			/* set flag */
			Flags[ip] = kIsElastic;
			
			return 0;
		}
	}
}	

/* Computes the stress corresponding to the given element
 * and elastic strain.  The function returns a reference to the
 * stress in fDevStress */
dSymMatrixT& DPSSLinHardT::DeviatoricStress(const dSymMatrixT& trialstrain,
	const ElementCardT& element)
{
#pragma unused(element)

	/* deviatoric strain */
	fDevStrain.Deviatoric(trialstrain);

	/* compute deviatoric elastic stress */
	fDevStress.SetToScaled(2.0*fmu,fDevStrain);

	return fDevStress;
}

/* computes the hydrostatic (mean) stress */
double DPSSLinHardT::MeanStress(const dSymMatrixT& trialstrain,
	const ElementCardT& element)
{
#pragma unused(element)

  fMeanStress = fkappa*trialstrain.Trace();
  return fMeanStress;
}
