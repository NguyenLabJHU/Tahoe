/* $Id: iNodeT.cpp,v 1.4 2002-07-02 19:57:24 cjkimme Exp $ */
/* created: paklein (12/07/1997) */

#include "iNodeT.h"
#include "ArrayT.h"

/* array behavior */

using namespace Tahoe;

const bool ArrayT<iNodeT>::fByteCopy = true;

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
