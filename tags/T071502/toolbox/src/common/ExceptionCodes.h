/* $Id: ExceptionCodes.h,v 1.4 2002-07-05 22:26:23 paklein Exp $ */
/* created: paklein (06/04/1996)                                          */

#ifndef _EXCEPTION_CODES_H_
#define _EXCEPTION_CODES_H_

namespace Tahoe {

/* number of exception codes */
const int eNumExceptions	= 10;

/* exception codes */
const int eNoError          = 0; // no error
const int eGeneralFail		= 1; // general unrecoverable error
const int eStop             = 2; // stop
const int eOutOfMemory      = 3; // out of memory
const int eOutOfRange       = 4; // index range error
const int eSizeMismatch     = 5; // (array) dimension mismatch
const int eBadInputValue    = 6; // bad input/construction parameter
const int eBadJacobianDet	= 7; // ParentDomainT:bad jacobian determinant
const int eMPIFail          = 8; // general error on MPI call
const int eDatabaseFail     = 9; // general error reading/writing database

} // namespace Tahoe 
#endif /* _EXCEPTION_CODES_H_ */