/* $Id: iNodeT.cpp,v 1.6 2003-10-27 19:51:01 paklein Exp $ */
/* created: paklein (12/07/1997) */
#include "iNodeT.h"
#include "ArrayT.h"

using namespace Tahoe;

/* array behavior */
namespace Tahoe {
template<> const bool ArrayT<iNodeT>::fByteCopy = true;
} /* namespace Tahoe */

/* constructors */
#ifndef __MWERKS__ /* VC++ doesn't like inline constructors */
iNodeT::iNodeT(void): fCoords(0), fTag(-1) { }
iNodeT::iNodeT(double* coords, int n) { Set(coords,n); }
#endif /* __MWERKS__ */

/* make blank */
void iNodeT::Clear(void)
{
	fCoords = 0;
	fTag    =-1;
}
