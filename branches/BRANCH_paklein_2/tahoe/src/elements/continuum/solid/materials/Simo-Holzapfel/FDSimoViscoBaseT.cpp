/* $Id: FDSimoViscoBaseT.cpp,v 1.2.2.2 2002-11-13 08:44:16 paklein Exp $ */
/* created:   TDN (5/31/2001) */
#include "FDSimoViscoBaseT.h"

#include "fstreamT.h"
#include "ExceptionT.h"
//#include "ContinuumElementT.h"
#include "ElementSupportT.h"

using namespace Tahoe;

const int kNumOutputVar = 2;
static const char* Labels[kNumOutputVar] = {"r_dil","r_dev"};

FDSimoViscoBaseT::FDSimoViscoBaseT(ifstreamT& in,  
				   const FDMatSupportT& support):
	FDStructMatT(in, support)
{
	int nsd = NumSD();
        int numstress = (nsd*(nsd+1))/2;
		
	/*allocates storage for state variable array*/
	fnstatev = 0;
	fnstatev += numstress;   /*current deviatoric overstress*/
	fnstatev += numstress;   /*current deviatoric inelastic stress*/
	fnstatev ++;			 /*current mean over stress*/
	fnstatev ++; 			 /*current mean inelastic stress*/
	
	fnstatev += numstress;   /*preceding deviatoric overstress*/
	fnstatev += numstress;   /*preceding deviatoric inelastic stress*/
	fnstatev ++;			 /*preceding mean overstress*/
	fnstatev ++; 			 /*preceding mean inelastic stress*/
	
	fstatev.Dimension(fnstatev);
	double* pstatev = fstatev.Pointer();
	/* assign pointers to current and preceding blocks of state variable array */
	
	fDevOverStress.Set(nsd, pstatev);        
	pstatev += numstress;
	fDevInStress.Set(nsd, pstatev);	
	pstatev += numstress;
	fVolOverStress.Set(1, pstatev);
	pstatev ++;
	fVolInStress.Set(1, pstatev);
	pstatev ++;
	
	fDevOverStress_n.Set(nsd, pstatev);        
	pstatev += numstress;
	fDevInStress_n.Set(nsd, pstatev);	
	pstatev += numstress;
	fVolOverStress_n.Set(1, pstatev);
	pstatev ++;
	fVolInStress_n.Set(1, pstatev);
}	

void FDSimoViscoBaseT::Print(ostream& out) const
{
	/* inherited */
	FDStructMatT::Print(out);
}

void FDSimoViscoBaseT::PrintName(ostream& out) const
{
	/* inherited */
	FDStructMatT::PrintName(out);
        out << "Finite Deformation Simo Viscoelastic Model \n";
}

void FDSimoViscoBaseT::PointInitialize(void)
{
	/* allocate element storage */
	if (CurrIP() == 0)
	{
		ElementCardT& element = CurrentElement();
		element.Dimension(0, fnstatev*NumIP());
	
	/* initialize internal variables to 0.0*/
		element.DoubleData() = 0.0;
	}

	/* store results as last converged */
	if (CurrIP() == NumIP() - 1) 
		UpdateHistory();
}
 
void FDSimoViscoBaseT::UpdateHistory(void)
{
	/* current element */
	ElementCardT& element = CurrentElement();	
	for (int ip = 0; ip < NumIP(); ip++)
	{
		/* load state variables */
		Load(element, ip);
	
		/* assign "current" to "last" */	
		fDevOverStress_n = fDevOverStress;
		fDevInStress_n = fDevInStress;
		fVolOverStress_n = fVolOverStress;
		fVolInStress_n = fVolInStress;

		/* write to storage */
		Store(element, ip);
	}
}

void FDSimoViscoBaseT::ResetHistory(void)
{
	/* current element */
	ElementCardT& element = CurrentElement();	
	for (int ip = 0; ip < NumIP(); ip++)
	{
		/* load state variables*/
		Load(element, ip);
	
		/* assign "last" to "current" */
		fDevOverStress = fDevOverStress_n;
		fDevInStress = fDevInStress_n;
		fVolOverStress = fVolOverStress_n;
		fVolInStress = fVolInStress_n;
		
		/* write to storage */
		Store(element, ip);
	}
}

void FDSimoViscoBaseT::Load(ElementCardT& element, int ip)
{
	/* fetch internal variable array */
	dArrayT& d_array = element.DoubleData();

	/* copy/convert */
	double* pd = d_array.Pointer(fnstatev*ip);
	double* pdr = fstatev.Pointer();
	for (int i = 0; i < fnstatev; i++)
		*pdr++ = *pd++;
}

void FDSimoViscoBaseT::Store(ElementCardT& element, int ip)
{
	/* fetch internal variable array */
	dArrayT& d_array = element.DoubleData();

	/* copy/convert */
	double* pdr = fstatev.Pointer();
	double* pd = d_array.Pointer(fnstatev*ip);
	for (int i = 0; i < fnstatev; i++)
		*pd++ = *pdr++;
}

int FDSimoViscoBaseT::NumOutputVariables() const {return kNumOutputVar;}

void FDSimoViscoBaseT::OutputLabels(ArrayT<StringT>& labels) const
{
	//allocates space for labels
	labels.Dimension(kNumOutputVar);
	
	//copy labels
	for (int i = 0; i< kNumOutputVar; i++)
		labels[i] = Labels[i];
}
	
void FDSimoViscoBaseT::ComputeOutput(dArrayT& output)
{
  // obtains ratio of elastic strain to total strain
	
	ElementCardT& element = CurrentElement();
	Load(element, CurrIP());
	double p_over = fVolOverStress[0];
	double p_in = fVolInStress[0];
	double s_over;
	double s_in;
	/*equivalent deviatoric stress*/
	double third = 1.0/3.0;
	s_over = sqrt(2.0*third*fDevOverStress.ScalarProduct());
	s_in = sqrt(2.0*third*fDevInStress.ScalarProduct());

	double r_dil = p_over/p_in;
	double r_dev = s_over/s_in;

	if (r_dil > 1 || r_dil <0) 
	  r_dil = 0.0;

	if (r_dev > 1 || r_dev <0) 
	  r_dev = 0.0;

	output[0] = r_dil;
	output[1] = r_dev;
}	

double FDSimoViscoBaseT::StrainEnergyDensity(void)
{
        cout << "\nStrain Energy is undefined.";
	throw ExceptionT::kGeneralFail;
}