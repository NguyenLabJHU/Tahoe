/* $Id: ios_fwd_decl.h,v 1.2 2001-03-01 01:17:49 hspark Exp $ */
/* created: paklein (08/11/1999)                                          */
/* iosfwd.h                                                               */
/* include this header instead of writing forward declarations            */
/* explicitly. MSL does not allow forward declarations of stream          */
/* classes.                                                               */

#ifndef _IOSFWD_H_
#define _IOSFWD_H_

#include "Environment.h"

#ifdef _MW_MSL_ // Metrowerks Standard Library
#include <iosfwd.h> //MSL C++ header
#elif defined(__SUNPRO_CC) 
// SUNWspro 5.0
#include <iostream.h>
#include <fstream.h>
#elif defined(__GNU__) && defined (__PGI__)
//PAK(02/28/2001): this header does not seem to work
//#include <iosfwd.h>
#include <iostream.h>
#else // forward declarations OK
class istream;
class ostream;
class ifstream;
class ofstream;
#endif // _MW_MSL_

#endif // _IOSFWD_H_
