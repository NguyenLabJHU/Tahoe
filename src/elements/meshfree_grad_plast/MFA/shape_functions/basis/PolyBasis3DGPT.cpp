/* $Id: PolyBasis3DGPT.cpp,v 1.2 2004-07-06 20:16:24 kyonten Exp $ */
/* created: paklein (04/19/2000)                                          */

#include "PolyBasis3DGPT.h"

/* constructor */

using namespace Tahoe;

PolyBasis3DGPT::PolyBasis3DGPT(int complete):
	BasisGPT(complete, 3)
{
	if (fComplete < 0 || fComplete > 1)
	{
		cout << "\n PolyBasis3DGPT::PolyBasis3DGPT: completeness must be [0,1]" << endl;
		throw ExceptionT::kBadInputValue;	
	}
}
	
/* return the number of basis functions */
int PolyBasis3DGPT::BasisDimension(void) const
{
	switch (fComplete)
	{
		case 0:			
			return 1;
		case 1:
			return 4;
		case 2:
			return 10;
		default:
			throw ExceptionT::kOutOfRange;
	}
	return 0;
}

/* evaluate basis functions at coords */
void PolyBasis3DGPT::SetBasis(const dArray2DT& coords, int order)
{
#if __option(extended_errorcheck)
	/* dimension checking */
	if (coords.MinorDim() != fNumSD) throw ExceptionT::kGeneralFail;
	if (order > 2) throw ExceptionT::kOutOfRange;
#endif

	/* dimensions */
	int nnd = coords.MajorDim();

	/* dimension work space */
	Dimension(nnd);
	switch (fComplete)
	{
		case 0: // constant basis
		{
			fP = 1.0;
			if (order > 0)
			{
				fDP[0] = 0.0;
				fDP[1] = 0.0;
				fDP[2] = 0.0;
				if (order > 1)
				{
					fDDP[0] = 0.0;
					fDDP[1] = 0.0;
					fDDP[2] = 0.0;
					fDDP[3] = 0.0;
					fDDP[4] = 0.0;
					fDDP[5] = 0.0;
					if (order > 2) // kyonten
					{
						fDDDP[0] = 0.0;
						fDDDP[1] = 0.0;
						fDDDP[2] = 0.0;
						fDDDP[3] = 0.0;
						fDDDP[4] = 0.0;
						fDDDP[5] = 0.0;
						fDDDP[6] = 0.0;
						fDDDP[7] = 0.0;
						fDDDP[8] = 0.0;
						fDDDP[9] = 0.0;
					}
				}
			}
			break;
		}
		case 1: // linear basis
		{
			const double* px = coords.Pointer();
			
			double*  pP0 = fP(0);
			double*  pP1 = fP(1);
			double*  pP2 = fP(2);
			double*  pP3 = fP(3);

			double* pD0P0 = (fDP[0])(0);
			double* pD0P1 = (fDP[0])(1);
			double* pD0P2 = (fDP[0])(2);
			double* pD0P3 = (fDP[0])(3);

			double* pD1P0 = (fDP[1])(0);
			double* pD1P1 = (fDP[1])(1);
			double* pD1P2 = (fDP[1])(2);
			double* pD1P3 = (fDP[1])(3);

			double* pD2P0 = (fDP[2])(0);
			double* pD2P1 = (fDP[2])(1);
			double* pD2P2 = (fDP[2])(2);
			double* pD2P3 = (fDP[2])(3);
			for (int i = 0; i < nnd; i++)
			{
				*pP0++ = 1.0;
				*pP1++ = *px++;
				*pP2++ = *px++;
				*pP3++ = *px++;

				if (order > 0)
				{
					*pD0P0++ = 0.0;
					*pD0P1++ = 1.0;
					*pD0P2++ = 0.0;
					*pD0P3++ = 0.0;

					*pD1P0++ = 0.0;
					*pD1P1++ = 0.0;
					*pD1P2++ = 1.0;
					*pD1P3++ = 0.0;

					*pD2P0++ = 0.0;
					*pD2P1++ = 0.0;
					*pD2P2++ = 0.0;
					*pD2P3++ = 1.0;
				}
			}
			if (order > 1)
			{
				fDDP[0] = 0.0;
				fDDP[1] = 0.0;
				fDDP[2] = 0.0;
				fDDP[3] = 0.0;
				fDDP[4] = 0.0;
				fDDP[5] = 0.0;
			}
			if (order > 2) // kyonten
			{
				fDDDP[0] = 0.0;
				fDDDP[1] = 0.0;
				fDDDP[2] = 0.0;
				fDDDP[3] = 0.0;
				fDDDP[4] = 0.0;
				fDDDP[5] = 0.0;
				fDDDP[6] = 0.0;
				fDDDP[7] = 0.0;
				fDDDP[8] = 0.0;
				fDDDP[9] = 0.0;
			}
			break;
		}
	}
}
