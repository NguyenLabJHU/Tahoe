/* $Id: LocalArrayT.cpp,v 1.11.2.1 2002-10-17 01:51:26 paklein Exp $ */
/* created: paklein (07/10/1996) */

#include "LocalArrayT.h"
#include "dArray2DT.h"
#include "iArrayT.h"

/* array behavior */

using namespace Tahoe;

namespace Tahoe {
const bool ArrayT<LocalArrayT::TypeT>::fByteCopy = true;
} /* namespace Tahoe */

/* cconstructors */
LocalArrayT::LocalArrayT(void):
	fType(kUnspecified),
	fNumNodes(0),
	fMinorDim(0),
	fGlobal(NULL)
{

}

LocalArrayT::LocalArrayT(TypeT type):
	fType(type),
	fNumNodes(0),
	fMinorDim(0),
	fGlobal(NULL)
{

}

LocalArrayT::LocalArrayT(TypeT type, int numnodes, int minordim):
	fType(type),
	fGlobal(NULL)
{
	Dimension(numnodes, minordim);
}

LocalArrayT::LocalArrayT(const LocalArrayT& source):
	fType(source.fType)
{
	*this = source;
}

/* copy data from an nArrayT */
void LocalArrayT::Copy(int numnodes, int minordim, const nArrayT<double>& source)
{
#if __option(extended_errorcheck)
	if (numnodes*minordim != source.Length()) throw ExceptionT::kSizeMismatch;
#endif

	/* dimensions */
	fNumNodes = numnodes;
	fMinorDim = minordim;
	if (fGlobal && fMinorDim != fGlobal->MinorDim()) throw ExceptionT::kSizeMismatch;

	/* inherited */
	nArrayT<double>::operator=(source);
}

/* assignment operator */
LocalArrayT& LocalArrayT::operator=(const LocalArrayT& RHS)
{
	/* inherited */
	dArrayT::operator=(RHS);

	/* set dimensions */
	fNumNodes = RHS.fNumNodes;
	fMinorDim = RHS.fMinorDim;

	/* source */
	fGlobal = RHS.fGlobal;
	
	return *this;
}

/* combining arrays - inserts all of source at start_node */
void LocalArrayT::BlockCopyAt(const LocalArrayT& source, int start_node)
{
#if __option (extended_errorcheck)
	if (source.MinorDim() != MinorDim()) throw ExceptionT::kSizeMismatch;
	if (start_node < 0 ||
	    start_node + source.NumberOfNodes() > NumberOfNodes())
	    throw ExceptionT::kOutOfRange;
#endif

	int size = sizeof(double)*source.NumberOfNodes();
	for (int i = 0; i < fMinorDim; i++)
		memcpy((*this)(i) + start_node, source(i), size);
}

/* compute the array average value */
void LocalArrayT::Average(dArrayT& avg) const
{
	/* dimension */
	avg.Dimension(MinorDim());
	avg = 0;
	for (int i = 0; i < MinorDim(); i++)
	{
		double* p = (*this)(i);
		double& s = avg[i];
		for (int j = 0; j < NumberOfNodes(); j++)
			s += *p++;
	}
	avg /= NumberOfNodes();
}

/* return the vector with transposed indexing */
void LocalArrayT::ReturnTranspose(nArrayT<double>& transpose) const
{
#if __option (extended_errorcheck)
	/* dimensions check */
	if(fLength != transpose.Length()) throw ExceptionT::kSizeMismatch;
#endif

	double* ptrans = transpose.Pointer();
	for (int i = 0; i < fNumNodes; i++)
	{
		double* pthis = fArray + i;
		for (int j = 0; j < fMinorDim; j++)
		{
			*ptrans++ = *pthis;
			pthis += fNumNodes;
		}
	}
}

void LocalArrayT::FromTranspose(const double* transpose)
{
	for (int i = 0; i < fNumNodes; i++)
	{
		double* pthis = fArray + i;	
		for (int j = 0; j < fMinorDim; j++)
		{
			*pthis = *transpose++;
			pthis += fNumNodes;
		}
	}
}

void LocalArrayT::AddScaledTranspose(double scale, const nArrayT<double>& transpose)
{
#if __option (extended_errorcheck)
	/* dimension check */
	if(fLength != transpose.Length()) throw ExceptionT::kSizeMismatch;
#endif

	double* ptrans = transpose.Pointer();
	for (int i = 0; i < fNumNodes; i++)
	{
		double* pthis = fArray + i;	
		for (int j = 0; j < fMinorDim; j++)
		{
			*pthis += scale*(*ptrans++);
			pthis += fNumNodes;
		}
	}
}

/* for registered arrays - preset source for SetLocal */
void LocalArrayT::SetGlobal(const dArray2DT& global)
{
#if __option(extended_errorcheck)
	if (global.MinorDim() != fMinorDim) throw ExceptionT::kSizeMismatch;
#endif
	
	fGlobal = &global;
}

void LocalArrayT::SetLocal(const ArrayT<int>& keys)
{
#if __option (extended_errorcheck)
	if (!fGlobal) throw ExceptionT::kGeneralFail;
	if (keys.Length() != fNumNodes) throw ExceptionT::kSizeMismatch;
#endif

	fGlobal->SetLocal(keys,*this);
}
