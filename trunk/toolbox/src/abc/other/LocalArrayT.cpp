/* $Id: LocalArrayT.cpp,v 1.5 2002-02-18 08:48:42 paklein Exp $ */
/* created: paklein (07/10/1996) */

#include "LocalArrayT.h"
#include "dArray2DT.h"
#include "iArrayT.h"

/* array behavior */
const bool ArrayT<LocalArrayT::TypeT>::fByteCopy = true;

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
	if (source.MinorDim() != MinorDim()) throw eSizeMismatch;
	if (start_node < 0 ||
	    start_node + source.NumberOfNodes() > NumberOfNodes())
	    throw eOutOfRange;
#endif

	int size = sizeof(double)*source.NumberOfNodes();
	for (int i = 0; i < fMinorDim; i++)
		memcpy((*this)(i) + start_node, source(i), size);
}

/* return the vector with transposed indexing */
void LocalArrayT::ReturnTranspose(nArrayT<double>& transpose) const
{
#if __option (extended_errorcheck)
	/* dimensions check */
	if(fLength != transpose.Length()) throw eSizeMismatch;
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
	if(fLength != transpose.Length()) throw eSizeMismatch;
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
	if (global.MinorDim() != fMinorDim) throw eSizeMismatch;
#endif
	
	fGlobal = &global;
}

void LocalArrayT::SetLocal(const ArrayT<int>& keys)
{
#if __option (extended_errorcheck)
	if (!fGlobal) throw eGeneralFail;
	if (keys.Length() != fNumNodes) throw eSizeMismatch;
#endif

	fGlobal->SetLocal(keys,*this);
}
