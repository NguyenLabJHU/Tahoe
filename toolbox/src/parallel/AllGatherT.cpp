/* $Id: AllGatherT.cpp,v 1.1.2.1 2002-12-10 17:01:17 paklein Exp $ */
#include "AllGatherT.h"
#include "CommunicatorT.h"

using namespace Tahoe;

/* constructor */
AllGatherT::AllGatherT(CommunicatorT& comm):
	MessageT(comm),
	fEqual(false),
	fTotal(0)
{

}

/* initialize gather data */
void AllGatherT::Initialize(int my_size)
{
	/* get counts from all */
	fCounts.Dimension(fComm.Size());
	fCounts = 0;
	
	/* collect counts from all */
	fComm.AllGather(my_size, fCounts);
	fTotal = fCounts.Sum();
	
	/* determine if data size is the same from all */
	fEqual = Same(fCounts);
	
	/* set displacements */
	if (fEqual)
	{
		fDisplacements.Dimension(fCounts);
		int offset = 0;
		for (int i = 0; i < fCounts.Length(); i++)
		{
			fDisplacements[i] = offset;
			offset += fCounts[i];
		}
	}
	else fDisplacements.Dimension(0);
}

void AllGatherT::AllGather(const nArrayT<double>& my_data, nArrayT<double>& gather)
{
	/* check */
	if (gather.Length() < fTotal) ExceptionT::SizeMismatch("AllGatherT::AllGather");
	
	/* equal sized or not */
	if (fEqual)
		fComm.AllGather(my_data, gather);
	else
		fComm.AllGather(fCounts, fDisplacements, my_data, gather);
}

void AllGatherT::AllGather(const nArrayT<int>& my_data, nArrayT<int>& gather)
{
	/* check */
	if (gather.Length() < fTotal) ExceptionT::SizeMismatch("AllGatherT::AllGather");

	/* equal sized or not */
	if (fEqual)
		fComm.AllGather(my_data, gather);
	else
		fComm.AllGather(fCounts, fDisplacements, my_data, gather);
}
