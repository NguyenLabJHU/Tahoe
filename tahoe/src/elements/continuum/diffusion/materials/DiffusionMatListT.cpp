/* $Id: DiffusionMatListT.cpp,v 1.3 2002-10-20 22:49:02 paklein Exp $ */
/* created: paklein (02/14/1997)                                          */

#include "DiffusionMatListT.h"

#include "ContinuumElementT.h"
#include "fstreamT.h"

/* diffusion materials */
#include "DiffusionMaterialT.h"

/* diffusion materials */

using namespace Tahoe;

const int kLinear      = 1;

const int kMaterialMin = 1;
const int kMaterialMax = 1;

/* constructors */
DiffusionMatListT::	DiffusionMatListT(int length,
	const DiffusionT& element_group):
	MaterialListT(length),
	fElementGroup(element_group)
{

}

/* read material data from the input stream */
void DiffusionMatListT::ReadMaterialData(ifstreamT& in)
{
	/* read material data */
	for (int i = 0; i < fLength; i++)
	{
		int matnum, matcode;
		in >> matnum; matnum--;
		in >> matcode;
		
		/* checks */
		if (matnum < 0  || matnum >= fLength) throw ExceptionT::kBadInputValue;
		if (matcode < kMaterialMin ||
		    matcode > kMaterialMax) throw ExceptionT::kBadInputValue;

		/* repeated material number */
		if (fArray[matnum] != NULL)
		{
			cout << "\n DiffusionMatListT::ReadMaterialData: repeated material number: ";
			cout << matnum + 1 << endl;
			throw ExceptionT::kBadInputValue;
		}
		
		/* add to the list of materials */
		switch (matcode)
		{
			case kLinear:
			{
				fArray[matnum] = new DiffusionMaterialT(in, fElementGroup);
				break;
			}
			default:
			
				cout << "\n DiffusionMatListT::ReadMaterialData: unknown material code: ";
				cout << matcode << '\n' << endl;
				throw ExceptionT::kBadInputValue;
		}

		/* verify construction */
		if (!fArray[matnum]) throw ExceptionT::kOutOfMemory;
	}
}
