/* $Id: ExceptionT.cpp,v 1.5 2002-11-22 02:04:28 paklein Exp $ */
#include "ExceptionT.h"
#include "ArrayT.h"
#include <iostream.h>
#include <iomanip.h>
#include <time.h>

#ifdef __SGI__
#include <stdio.h>
#include <stdarg.h>
#else
#include <cstdio>
#include <cstdarg> /* to handle the variable number of arguments in Throw() */
#endif

/* initialize static data */
namespace Tahoe {
int ExceptionT::NumExceptions = 11;
const bool ArrayT<ExceptionT::CodeT>::fByteCopy = true;

/* exceptions strings */
const char* ExceptionT::fExceptionStrings[12] = 
{
/* 0 */ "no error",
/* 1 */ "general fail",
/* 2 */ "stop",
/* 3 */ "out of memory",
/* 4 */ "index out of range",
/* 5 */ "dimension mismatch",
/* 6 */ "invalid value read from input",
/* 7 */ "zero or negative jacobian",
/* 8 */ "MPI message passing error",
/* 9 */ "database read failure",
/*10 */ "bad MP heartbeat",
/*11 */ "unknown"};

/* buffer for vsprintf */
static char message_buffer[255];

/* write exception codes to output stream */
void ExceptionT::WriteExceptionCodes(ostream& out)
{
	out << "\nE x c e p t i o n   c o d e s :\n\n";	
	for (int i = 0; i < NumExceptions; i++)
	{
		out << setw(kIntWidth) << i << " : ";
		out << fExceptionStrings[i] << '\n';
	}	
		out << endl;
}

/* return exception string */
const char* ExceptionT::ToString(CodeT code)
{
	if (code >= 0 && code < NumExceptions)
		return fExceptionStrings[code];
	else
		return fExceptionStrings[NumExceptions];
}

/* repeated here instead of passing a va_list to Throw so that the
 * cstdarg header is limited to this source file */
void ExceptionT::GeneralFail(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kGeneralFail, caller, message_buffer);
}

void ExceptionT::Stop(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kStop, caller, message_buffer);
}

void ExceptionT::OutOfMemory(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kOutOfMemory, caller, message_buffer);
}

void ExceptionT::OutOfRange(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kOutOfRange, caller, message_buffer);
}

void ExceptionT::SizeMismatch(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kSizeMismatch, caller, message_buffer);
}

void ExceptionT::BadInputValue(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kBadInputValue, caller, message_buffer);
}

void ExceptionT::BadJacobianDet(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kBadJacobianDet, caller, message_buffer);
}

void ExceptionT::MPIFail(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kMPIFail, caller, message_buffer);
}

void ExceptionT::DatabaseFail(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kDatabaseFail, caller, message_buffer);
}

void ExceptionT::BadHeartBeat(const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(kBadHeartBeat, caller, message_buffer);
}

void ExceptionT::Throw(ExceptionT::CodeT code, const char* caller, const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsprintf(message_buffer, fmt, argp);
	va_end(argp);
	Throw_(code, caller, message_buffer);
}

void ExceptionT::Throw_(ExceptionT::CodeT code, const char* caller, const char* message)
{
	/* write info */
	time_t t;
	time(&t);
	cout << "\n ExceptionT::Throw: " << ctime(&t);
	cout <<   "        code: " << code << '\n';
	cout <<   "   exception: " << ToString(code) << '\n';
	if (caller)
	cout <<   "      caller: " << caller << '\n';
	if (message)
	cout <<   "     message: " << message << '\n';
	
	/* do the throw */
	throw code;
}

} /* namespace Tahoe */ 
