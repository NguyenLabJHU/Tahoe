/* $Id: RG_VDSplit3D.cpp,v 1.3 2002-11-14 17:06:09 paklein Exp $ */
/* created: TDN (01/22/2001) */

#include "fstreamT.h"
#include "ExceptionT.h"
#include "RG_VDSplit3D.h"

#include <math.h>
#include <iostream.h>
#include <stdlib.h>

using namespace Tahoe;
 
const int kNumOutputVar = 1; 
static const char* Labels[kNumOutputVar] = {"dW_visc"}; 

/***********************************************************************
 * Public
 ***********************************************************************/

/* constructors */
RG_VDSplit3D::RG_VDSplit3D(ifstreamT& in, const FDMatSupportT& support):
	RGBaseT(in, support),
	fb(3),
	fEigs(3),
	fEigs_e(3),
	fEigs_bar(3),
	fEigs_ebar(3),
	ftau_EQ(3),
	ftau_NEQ(3),
	fDtauDe_EQ(3),
	fDtauDe_NEQ(3),
	fModMat(6),
	fModulus(6),
	fStress(3),
	fiKAB(3,3),
	fthird(1.0/3.0)
{
	/*read in viscosities*/
        double A;
	in >> A;
	fietaS = 1.0/A;
	in >> A;
	fietaB = 1.0/A;
}

/* print parameters */
void RG_VDSplit3D::Print(ostream& out) const
{
        RGBaseT::Print(out);

	out<<"Constant Viscosity \n";
	out<<"     Shear Viscosity: "<<1.0/fietaS<<'\n';
	out<<"     Bulk Viscosity: "<<1.0/fietaB<<'\n';
}

/* print name */
void RG_VDSplit3D::PrintName(ostream& out) const
{
	/* inherited */
	RGBaseT::PrintName(out);
}

int RG_VDSplit3D::NumOutputVariables() const {return kNumOutputVar;} 
 
void RG_VDSplit3D::OutputLabels(ArrayT<StringT>& labels) const 
{ 
  //allocates space for labels 
        labels.Dimension(kNumOutputVar); 
   
        //copy labels 
        for (int i = 0; i< kNumOutputVar; i++) 
                labels[i] = Labels[i]; 
} 

double RG_VDSplit3D::StrainEnergyDensity(void)
{
	/*calculates deviatoric and volumetric part of the total stretch */
	Compute_b(fb);
	fb.PrincipalValues(fEigs);
	double J = sqrt(fEigs[0]*fEigs[1]);
	dArrayT eigenstretch_bar = fEigs;
	eigenstretch_bar *= pow(J, -2.0*fthird);

	double energy =0.0;
	energy = Phi(eigenstretch_bar, J, kEquilibrium);

	/*calculates deviatoric and volumetric part of the elastic stretch */
	ElementCardT& element = CurrentElement();
	Load(element, CurrIP());

	fSpectralDecompRef.SpectralDecomp_Jacobi(fC_v, false);	
	dArrayT Eigs_v(2);
	fC_v.PrincipalValues(Eigs_v);

	fEigs_e = fEigs;
	fEigs_e /= Eigs_v;
	
	J = sqrt(fEigs_e[0]*fEigs_e[1]);
	eigenstretch_bar.SetToScaled(pow(J,-2.0*fthird), fEigs_e);

	energy += Phi(eigenstretch_bar, J,kNonEquilibrium);

	return(energy);
}

/* modulus */
const dMatrixT& RG_VDSplit3D::c_ijkl(void)
{
        /*assumes s_ij() has been called*/

        double iJ=1.0/fJ;
  
	DdevTauDepsilon(fEigs_bar, fDtauDe_EQ, kEquilibrium);
	fDtauDe_EQ += DmeanTauDepsilon(fJ, kEquilibrium);
	dSymMatrixT Gamma = fDtauDe_EQ;
	Gamma *= iJ;

        DdevTauDepsilon(fEigs_ebar, fDtauDe_NEQ, kNonEquilibrium);
	double cm = DmeanTauDepsilon(fJe, kNonEquilibrium);
	ComputeiKAB(fDtauDe_NEQ, cm);
	dSymMatrixT DAB = fDtauDe_NEQ;
	DAB += cm; 
	dMatrixT Calg(3);
	Calg.MultSymAB(DAB,fiKAB);
	Calg *= iJ;

	const ArrayT<dArrayT>& eigenvectors=fSpectralDecompSpat.Eigenvectors();
	const ArrayT<dArrayT>& eigenvectors2=fSpectralDecompRef.Eigenvectors();
	double dlamb, coeff;

	/*Assemble moduli*/
	/*axial*/
	Gamma(0,0) -= 2.0*iJ*ftau_EQ[0];
	Gamma(1,1) -= 2.0*iJ*ftau_EQ[1];
	Gamma(2,2) -= 2.0*iJ*ftau_EQ[2];

	Calg(0,0) -= 2.0*iJ*ftau_NEQ[0];
	Calg(1,1) -= 2.0*iJ*ftau_NEQ[1];
	Calg(2,2) -= 2.0*iJ*ftau_NEQ[2];
	
	fModulus = fSpectralDecompSpat.EigsToRank4(Gamma);	
	fModulus += fSpectralDecompSpat.NonSymEigsToRank4(Calg);
	
	double sig1 = iJ*(ftau_EQ[0]+ftau_NEQ[0]);
	double sig2 = iJ*(ftau_EQ[1]+ftau_NEQ[1]);
	double sig3 = iJ*(ftau_EQ[2]+ftau_NEQ[2]);
	
	double& lamb1 = fEigs[0];
	double& lamb2 = fEigs[1];
	double& lamb3 = fEigs[2];
	
	/* 1,2 */
	dlamb = lamb1 - lamb2;
	/* modulus coefficient */
	if (fabs(dlamb) > kSmall)
	  coeff = (sig1*lamb2 - sig2*lamb1)/dlamb;
	else
	  coeff = 0.5*(Gamma(0,0) - Gamma(0,1) + 
		       Calg(0,0) - Calg(0,1));
	MixedRank4_3D(eigenvectors[0], eigenvectors[1], fModMat);
	fModulus.AddScaled(2.0*coeff, fModMat);
	
	/* 1,3 */
	dlamb = lamb1 - lamb3;
	/* modulus coefficient */
	if (fabs(dlamb) > kSmall)
	  coeff = (sig1*lamb3 - sig3*lamb1)/dlamb;
	else
	  coeff = 0.5*(Gamma(0,0) - Gamma(0,2) + 
		       Calg(0,0) - Calg(0,2));	
	MixedRank4_3D(eigenvectors[0], eigenvectors[2], fModMat);
	fModulus.AddScaled(2.0*coeff, fModMat);
	
	/* 2,3 */
	dlamb = lamb2 - lamb3;
	/* modulus coefficient */
	if (fabs(dlamb) > kSmall)
	  coeff = (sig2*lamb3 - sig3*lamb2)/dlamb;
	else
	  coeff = 0.5*(Gamma(1,1) - Gamma(1,2) + 
		       Calg(1,1) - Calg(1,2));	
	MixedRank4_3D(eigenvectors[1], eigenvectors[2], fModMat);
	fModulus.AddScaled(2.0*coeff, fModMat);

	return fModulus;
}

/* stresses */
const dSymMatrixT& RG_VDSplit3D::s_ij(void)
{
	/* stretch tensor */
	Compute_b(fb);

	/* spectral decomposition */
	fSpectralDecompSpat.SpectralDecomp_Jacobi(fb, false);	
	/*calculates principal stretch*/
	
	fEigs = fSpectralDecompSpat.Eigenvalues();
	fJ = sqrt(fEigs[0]*fEigs[1]*fEigs[2]);
	fEigs_bar.SetToScaled(pow(fJ,-2.0*fthird),fEigs);

	devTau(fEigs_bar, ftau_EQ, kEquilibrium);
	ftau_EQ += meanTau(fJ, kEquilibrium);
	
	/*load the viscoelastic principal stretches from state variable arrays*/
	ElementCardT& element = CurrentElement();
	Load(element, CurrIP());
	if (fFDMatSupport.RunState() == GlobalT::kFormRHS)
	{
		dSymMatrixT iCvn = fC_vn;
		iCvn.Inverse();

		/*calculate trial state;*/
		dSymMatrixT b_tr(3);
		const dMatrixT& F = F_mechanical();
		b_tr.MultQBQT(F,iCvn);

		/*set initial value of elastic principal stretches 
		 * to trial values*/
		fSpectralDecompSpat.SpectralDecomp_Jacobi(b_tr, false);	
		fEigs_e = fSpectralDecompSpat.Eigenvalues();
		ComputeEigs_e(fEigs, fEigs_e, ftau_NEQ, fDtauDe_NEQ);

		/*update viscuous stretch tensor*/
		Compute_C(fC_v);
		fSpectralDecompRef.SpectralDecomp_Jacobi(fC_v,false);
		dArrayT Eigs_v = fEigs;
		Eigs_v /= fEigs_e;
		fC_v = fSpectralDecompRef.EigsToRank2(Eigs_v);
		Store(element, CurrIP());
	}	
	else 
	{
		fSpectralDecompRef.SpectralDecomp_Jacobi(fC_v, false);
	        dArrayT Eigs_v = fSpectralDecompRef.Eigenvalues();

		fEigs_e = fEigs;
		fEigs_e /= Eigs_v;
	}
	fJe = sqrt(fEigs_e[0]*fEigs_e[1]*fEigs_e[2]);
	fEigs_ebar.SetToScaled(pow(fJe,-2.0*fthird),fEigs_e);
	devTau(fEigs_ebar, ftau_NEQ, kNonEquilibrium);
	ftau_NEQ += meanTau(fJe, kNonEquilibrium);

	fSpectralDecompSpat.SpectralDecomp_Jacobi(fb, false);	
	fStress = fSpectralDecompSpat.EigsToRank2(ftau_EQ);
	fStress += fSpectralDecompSpat.EigsToRank2(ftau_NEQ);

	double iJ = 1.0/fJ;
	fStress *= iJ;
	return fStress;
}

/* material description */
const dMatrixT& RG_VDSplit3D::C_IJKL(void)
{
	/* deformation gradient */
	const dMatrixT& Fmat = F();
	
	/* transform */
	fModulus.SetToScaled(fJ, PullBack(Fmat, c_ijkl()));
	return fModulus;	
}

const dSymMatrixT& RG_VDSplit3D::S_IJ(void)
{
	/* deformation gradient */
	const dMatrixT& Fmat = F();
	
	/* transform */
	fStress.SetToScaled(fJ, PullBack(Fmat, s_ij()));
	return fStress;
}

void RG_VDSplit3D::ComputeOutput(dArrayT& output)
{
	/* stretch tensor */
	/* stretch tensor */
	Compute_b(fb);

	/* spectral decomposition */
	fSpectralDecompSpat.SpectralDecomp_Jacobi(fb, false);	
	/*calculates principal stretch*/
	
	fEigs = fSpectralDecompSpat.Eigenvalues();
	/*calc jacobian*/
	fJ = sqrt(fEigs.Product()) ;
	fEigs_bar.SetToScaled(pow(fJ,-2.0*fthird),fEigs);

	double iJ = 1.0/fJ;
	/*load the viscoelastic principal stretches from state variable arrays*/
	ElementCardT& element = CurrentElement();
	Load(element, CurrIP());
	fSpectralDecompRef.SpectralDecomp_Jacobi(fC_v, false);	
	dArrayT Eigs_v = fSpectralDecompRef.Eigenvalues();

	fEigs_e = fEigs;
	fEigs_e /= Eigs_v;
	fJe = sqrt(fEigs_e[0]*fEigs_e[1]*fEigs_e[2]);
	fEigs_ebar.SetToScaled(pow(fJe,-2.0*fthird),fEigs_e);

	dArrayT sig_NEQ(3);
	devTau(fEigs_ebar, sig_NEQ, kNonEquilibrium);
	sig_NEQ *=iJ;
	fSpectralDecompSpat.SpectralDecomp_Jacobi(fb, false);	
	fStress = fSpectralDecompSpat.EigsToRank2(sig_NEQ);

      	double sm = meanTau(fJ, kNonEquilibrium)*iJ;

	double rate_visc_disp = fStress.ScalarProduct()*0.5*fietaS+
	       fthird*sm*sm*fietaB;

	/*put in planestress option*/

	output[0] = rate_visc_disp*fFDMatSupport.TimeStep();
	
}
/***********************************************************************
 * Private
 ***********************************************************************/
void RG_VDSplit3D::ComputeEigs_e(const dArrayT& eigenstretch, 
			    dArrayT& eigenstretch_e, dArrayT& eigenstress, 
			    dSymMatrixT& eigenmodulus) 
{		
	const double ctol = 1.00e-10;
		
	/*set references to principle stretches*/
	double& l0 = eigenstretch[0];
	double& l1 = eigenstretch[1];
	double& l2 = eigenstretch[2];
      
	double& le0 = eigenstretch_e[0];
	double& le1 = eigenstretch_e[1];
	double& le2 = eigenstretch_e[2];
  
	double tol;

	/*initialize principle elastic and trial elastic log strains */
	double ep_tr0 = 0.5*log(le0);
	double ep_tr1 = 0.5*log(le1);
	double ep_tr2 = 0.5*log(le2);

	double ep_e0 = ep_tr0;		
	double ep_e1 = ep_tr1;	
	double ep_e2 = ep_tr2;

	/*initializes principle viscous stretch*/
	do 
	{

	        fJe=sqrt(le0*le1*le2);
	        fEigs_ebar.SetToScaled(pow(fJe,-2.0*fthird),
					    eigenstretch_e);

	        /*calculate stresses and moduli*/
		devTau(fEigs_ebar, eigenstress, kNonEquilibrium);

		double& s0 = eigenstress[0];
		double& s1 = eigenstress[1];
		double& s2 = eigenstress[2];

		DdevTauDepsilon(fEigs_ebar,eigenmodulus, kNonEquilibrium);

		/*caculate means*/
		double sm = meanTau(fJe, kNonEquilibrium);
		double cm = DmeanTauDepsilon(fJe, kNonEquilibrium);

 	  	ComputeiKAB(eigenmodulus,cm);
		
	   	/*calculate the residual*/
	   	double dt = fFDMatSupport.TimeStep();
	 	double res0 = ep_e0 + dt*(0.5*fietaS*s0 +
					fthird*fietaB*sm) - ep_tr0;
	 	double res1 = ep_e1 + dt*(0.5*fietaS*s1 +
					fthird*fietaB*sm) - ep_tr1;
	 	double res2 = ep_e2 + dt*(0.5*fietaS*s2 +
					fthird*fietaB*sm) - ep_tr2;
		
		//	cout << "\n residual1 "<< res0;
		/*solve for the principal strain increments*/
		double dep_e0=-fiKAB(0,0)*res0-fiKAB(0,1)*res1-fiKAB(0,2)*res2;
		double dep_e1=-fiKAB(1,0)*res0-fiKAB(1,1)*res1-fiKAB(1,2)*res2;
		double dep_e2=-fiKAB(2,0)*res0-fiKAB(2,1)*res1-fiKAB(2,2)*res2;
	 	
		/*updates principal elastic stretches*/ 
		ep_e0 += dep_e0;
		ep_e1 += dep_e1;
		ep_e2 += dep_e2;
		//	cout << "\n depsilon1 "<< dep_e0;

		le0 = exp(2.0*ep_e0);
		le1 = exp(2.0*ep_e1);
		le2 = exp(2.0*ep_e2);
		 
		/*Check that the L2 norm of the residual is less than tolerance*/
		tol = sqrt(res0*res0 + res1*res1+res2*res2);
	}while (tol>ctol); 
}

void RG_VDSplit3D::ComputeiKAB(dSymMatrixT& eigenmodulus, double& bulkmodulus)
{	
        /*inverse viscosities*/
        //		cout <<"\n Je: "<<Je;
	//		cout <<"\nfietaS: "<<fietaS;

	/*deviatoric values*/
	double& c0 = eigenmodulus(0,0);
	double& c1 = eigenmodulus(1,1);
	double& c2 = eigenmodulus(2,2);

	double& c12 = eigenmodulus(1,2);
	double& c02 = eigenmodulus(0,2);
	double& c01 = eigenmodulus(0,1);
	
	/*mean value*/
	double& cm = bulkmodulus;

	dMatrixT& KAB = fiKAB;
		
	/*calculates  KAB = 1+dt*D(dWdE_Idev/nD+isostress/nV)/Dep_e*/

	double dt = fFDMatSupport.TimeStep();
	KAB(0,0) = 1+0.5*fietaS*dt*c0+fthird*fietaB*dt*cm;
	KAB(1,1) = 1+0.5*fietaS*dt*c1+fthird*fietaB*dt*cm;
	KAB(2,2) = 1+0.5*fietaS*dt*c2+fthird*fietaB*dt*cm;

	KAB(1,2) = 0.5*fietaS*dt*c12+fthird*fietaB*dt*cm;
	KAB(0,2) = 0.5*fietaS*dt*c02+fthird*fietaB*dt*cm;
	KAB(0,1) = 0.5*fietaS*dt*c01+fthird*fietaB*dt*cm;
       
	KAB(2,1) = KAB(1,2);
	KAB(2,0) = KAB(0,2);
	KAB(1,0) = KAB(0,1);

	/*inverts KAB*/
	fiKAB.Inverse(KAB);
}

