 /* $Id: SSSimoViscoT.cpp,v 1.2 2003-04-05 20:38:07 thao Exp $ */
#include "SSSimoViscoT.h"
#include "fstreamT.h"
#include "ExceptionT.h"

using namespace Tahoe;

SSSimoViscoT::SSSimoViscoT(ifstreamT& in, const SSMatSupportT& support):
	SSSolidMatT(in, support)
{
	int ndof = 3;
	int numstress = (ndof*(ndof+1))/2;
		
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
	fnstatev += numstress;   /*viscous strains*/
	
	fstatev.Dimension(fnstatev);
	double* pstatev = fstatev.Pointer();
	/* assign pointers to current and preceding blocks of state variable array */
	
	fdevQ.Set(ndof, pstatev);        
	pstatev += numstress;
	fdevSin.Set(ndof, pstatev);	
	pstatev += numstress;
	fmeanQ.Set(1, pstatev);
	pstatev ++;
	fmeanSin.Set(1, pstatev);
	pstatev ++;
	
	fdevQ_n.Set(ndof, pstatev);        
	pstatev += numstress;
	fdevSin_n.Set(ndof, pstatev);	
	pstatev += numstress;
	fmeanQ_n.Set(1, pstatev);
	pstatev ++;
	fmeanSin_n.Set(1, pstatev);
	pstatev ++;
	
	fViscStrain.Set(ndof,pstatev);
	
}	

void SSSimoViscoT::Print(ostream& out) const
{
	/* inherited */
	SSSolidMatT::Print(out);
}

void SSSimoViscoT::PrintName(ostream& out) const
{
	/* inherited */
	SSSolidMatT::PrintName(out);
        out << "Finite Deformation Simo Viscoelastic Model \n";
}

void SSSimoViscoT::PointInitialize(void)
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
 
void SSSimoViscoT::UpdateHistory(void)
{
	/* current element */
	ElementCardT& element = CurrentElement();	
	for (int ip = 0; ip < NumIP(); ip++)
	{
		/* load state variables */
		Load(element, ip);
	
		/* assign "current" to "last" */	
		fdevQ_n = fdevQ;
		fdevSin_n = fdevSin;
		fmeanQ_n = fmeanQ;
		fmeanSin_n = fmeanSin;

		/* write to storage */
		Store(element, ip);
	}
}

void SSSimoViscoT::ResetHistory(void)
{
	/* current element */
	ElementCardT& element = CurrentElement();	
	for (int ip = 0; ip < NumIP(); ip++)
	{
		/* load state variables*/
		Load(element, ip);
	
		/* assign "last" to "current" */
		fdevQ = fdevQ_n;
		fdevSin = fdevSin_n;
		fmeanQ = fmeanQ_n;
		fmeanSin = fmeanSin_n;
		
		/* write to storage */
		Store(element, ip);
	}
}

void SSSimoViscoT::Load(ElementCardT& element, int ip)
{
	/* fetch internal variable array */
	dArrayT& d_array = element.DoubleData();

	/* copy/convert */
	double* pd = d_array.Pointer(fnstatev*ip);
	double* pdr = fstatev.Pointer();
	for (int i = 0; i < fnstatev; i++)
		*pdr++ = *pd++;
}

void SSSimoViscoT::Store(ElementCardT& element, int ip)
{
	/* fetch internal variable array */
	dArrayT& d_array = element.DoubleData();

	/* copy/convert */
	double* pdr = fstatev.Pointer();
	double* pd = d_array.Pointer(fnstatev*ip);
	for (int i = 0; i < fnstatev; i++)
		*pd++ = *pdr++;
}

