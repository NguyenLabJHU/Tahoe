/* $Id: iNodeT.cpp,v 1.3 2002-02-27 16:47:18 paklein Exp $ */
/* created: paklein (12/07/1997) */

#include "iNodeT.h"
#include "ArrayT.h"

/* array behavior */
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
