/* $Id: CommunicatorT.cpp,v 1.7.2.1 2002-12-10 17:03:41 paklein Exp $ */
#include "CommunicatorT.h"
#include "ExceptionT.h"
#include <iostream.h>
#include <time.h>
#include "nArrayT.h"

/* to handle the variable number of arguments in Throw() */
#if defined(__SGI__) || defined(__DELMAR__)
#include <stdio.h>
#include <stdarg.h>
#else
#include <cstdio>
#include <cstdarg>
#endif

/* buffer for vsprintf */
static char message_buffer[255];

using namespace Tahoe;

/* initialize static variables */
int CommunicatorT::fCount = 0;
int* CommunicatorT::fargc = NULL;
char*** CommunicatorT::fargv = NULL;

/* create communicator including all processes */
CommunicatorT::CommunicatorT(void):
	fLogLevel(kUrgent),
	fLog(&cout)
{
	/* check MPI environment */
	Init();

#ifdef __TAHOE_MPI__
	fComm = MPI_COMM_WORLD;
	if (MPI_Comm_size(fComm, &fSize) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::CommunicatorT", "MPI_Comm_size failed");

	if (MPI_Comm_rank(fComm, &fRank) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::CommunicatorT", "MPI_Comm_rank failed");
#else
	fComm = 0;
	fSize = 1;
	fRank = 0;
#endif
}

/* copy constructor */
CommunicatorT::CommunicatorT(const CommunicatorT& source):
	fComm(source.fComm),
	fSize(source.fSize),
	fRank(source.fRank),
	fLogLevel(source.fLogLevel),
	fLog(source.fLog)
{
	/* check MPI environment */
	Init();
}

/* destructor */
CommunicatorT::~CommunicatorT(void)
{
	/* check MPI environment */
	Finalize();
}

void CommunicatorT::Log(LogLevelT priority, const char* caller) const
{
	if (priority >= LogLevel())
	{
		/* log message */
		doLog(caller, NULL);

		/* throw exception */
		if (priority == kFail) ExceptionT::MPIFail(caller);
	}
}

void CommunicatorT::Log(LogLevelT priority, const char* caller, const char* fmt, ...) const
{
	if (priority >= LogLevel())
	{
		/* log message */
		va_list argp;
		va_start(argp, fmt);
		vsprintf(message_buffer, fmt, argp);
		va_end(argp);
		doLog(caller, message_buffer);

		/* throw exception */
		if (priority == kFail) ExceptionT::MPIFail(caller, message_buffer);
	}
}

/* (re-)set the logging stream */
void CommunicatorT::SetLog(ostream& log)
{
	Log(kLow, "SetLog", "closing log stream");
	fLog = &log;
	Log(kLow, "SetLog", "opening log stream");
}

/* maximum over single integers */
int CommunicatorT::Max(int a) const
{
	Log(kModerate, "Max", "in = %d", a);

#ifdef __TAHOE_MPI__
	if (Size() > 1)
	{
		int b = a;
		if (MPI_Allreduce(&b, &a, 1, MPI_INT, MPI_MAX, fComm) != MPI_SUCCESS)
			Log(kFail, "CommunicatorT::Max", "MPI_Allreduce failed");
	}
#endif

	Log(kModerate, "Max", "out = %d", a);

	return a;
}

/* minimum over single integers */
int CommunicatorT::Min(int a) const
{
	Log(kModerate, "Min", "in = %d", a);

#ifdef __TAHOE_MPI__
	if (Size() > 1)
	{
		int b = a;
		if (MPI_Allreduce(&b, &a, 1, MPI_INT, MPI_MIN, fComm) != MPI_SUCCESS)
			Log(kFail, "CommunicatorT::Min", "MPI_Allreduce failed");
	}
#endif

	Log(kModerate, "Min", "out = %d", a);
	return a;
}

/* minimum over single integers */
int CommunicatorT::Sum(int a) const
{
	Log(kModerate, "Sum", "in = %d", a);

#ifdef __TAHOE_MPI__
	if (Size() > 1)
	{
		int b = a;
		if (MPI_Allreduce(&b, &a, 1, MPI_INT, MPI_SUM, fComm) != MPI_SUCCESS)
			Log(kFail, "CommunicatorT::Sum", "MPI_Allreduce failed");
	}
#endif

	Log(kModerate, "Sum", "out = %d", a);
	return a;
}

/* maximum over single doubles */
double CommunicatorT::Max(double a) const
{
	Log(kModerate, "Max", "in = %e", a);

#ifdef __TAHOE_MPI__
	if (Size() > 1)
	{
		double b = a;
		if (MPI_Allreduce(&b, &a, 1, MPI_DOUBLE, MPI_MAX, fComm) != MPI_SUCCESS)
			Log(kFail, "CommunicatorT::Max", "MPI_Allreduce failed");
	}
#endif

	Log(kModerate, "Max", "out = %e", a);
	return a;
}

/* minimum over single doubles */
double CommunicatorT::Min(double a) const
{
	Log(kModerate, "Min", "in = %e", a);

#ifdef __TAHOE_MPI__
	if (Size() > 1)
	{
		double b = a;
		if (MPI_Allreduce(&b, &a, 1, MPI_DOUBLE, MPI_MIN, fComm) != MPI_SUCCESS)
			Log(kFail, "CommunicatorT::Min", "MPI_Allreduce failed");
	}
#endif

	Log(kModerate, "Min", "out = %e", a);
	return a;
}

/* minimum over single integers */
double CommunicatorT::Sum(double a) const
{
	Log(kModerate, "Sum", "in = %e", a);

#ifdef __TAHOE_MPI__
	if (Size() > 1)
	{
		double b = a;
		if (MPI_Allreduce(&b, &a, 1, MPI_DOUBLE, MPI_SUM, fComm) != MPI_SUCCESS)
			Log(kFail, "CommunicatorT::Sum", "MPI_Allreduce failed");
	}
#endif

	Log(kModerate, "Sum", "out = %e", a);
	return a;
}

/* gather single integer. Called by destination process. */
void CommunicatorT::Gather(int a, nArrayT<int>& gather) const
{
	Log(kModerate, "Gather", "in = %d, destination = %d", a, Rank());

	/* check */
	if (gather.Length() != Size()) ExceptionT::SizeMismatch("CommunicatorT::Gather");

	/* this */
	gather[Rank()] = a;

#ifdef __TAHOE_MPI__
	if (MPI_Gather(&a, 1, MPI_INT, gather.Pointer(), 1, MPI_INT, Rank(), fComm) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::Gather", "MPI_Gather failed");
#endif

	/* verbose */
	if (LogLevel() == kLow)
	{
		Log(kLow, "Gather", "destination");
		Log() << gather.wrap(8) << '\n';
	}
}

/* gather single integer. Called by sending processes. */
void CommunicatorT::Gather(int a, int destination) const
{
	Log(kModerate, "Gather", "in = %d, destination = %d", a, destination);

	/* check */
	if (destination == Rank())
		ExceptionT::GeneralFail("CommunicatorT::Gather", 
			"destination %d must be different from rank %d", destination, Rank());

#ifdef __TAHOE_MPI__
	int* tmp = NULL;
	if (MPI_Gather(&a, 1, MPI_INT, tmp, 1, MPI_INT, destination, fComm) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::Gather", "MPI_Gather failed");
#endif
}

/* synchronize all processes */
void CommunicatorT::Barrier(void) const
{
	Log(kLow, "Barrier");

#ifdef __TAHOE_MPI__
	if (MPI_Barrier(fComm) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::Barrier", "MPI_Barrier failed");
#endif

	Log(kLow, "Barrier");
}

/* gather single integer to all processes. */
void CommunicatorT::AllGather(int a, nArrayT<int>& gather) const
{
	Log(kModerate, "AllGather", "in = %d", a);

	/* check */
	if (gather.Length() != Size())
		ExceptionT::SizeMismatch("CommunicatorT::AllGather", 
			"buffer length %d does not match size %d", gather.Length(), Size());

	/* this */
	gather[Rank()] = a;

#ifdef __TAHOE_MPI__
	if (MPI_Allgather(&a, 1, MPI_INT, gather.Pointer(), 1, MPI_INT, fComm) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::Gather", "MPI_Allgather failed");
#endif

//NOTE: need to implement gather with sends and receives for CPLANT and other
//      platforms where Allgather seems to be troublesome.

	/* verbose */
	if (LogLevel() == kLow)
	{
		Log(kLow, "AllGather");
		Log() << gather.wrap(8) << '\n';
	}
}

/* gather multiple values from all */
void CommunicatorT::AllGather(const nArrayT<double>& my, nArrayT<double>& gather) const
{
	Log(kModerate, "AllGather", "sending %d", my.Length());
	if (LogLevel() == kLow) Log() << my.wrap(4) << '\n';

	/* check */
	if (my.Length()*Size() != gather.Length()) 
		ExceptionT::SizeMismatch("CommunicatorT::AllGather", 
			"buffer length %d does not match size %d", gather.Length(), my.Length()*Size());

#ifdef __TAHOE_MPI__
	int len = my.Length();
	if (MPI_Allgather(my.Pointer(), len, MPI_DOUBLE, gather.Pointer(), len, MPI_DOUBLE, fComm) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::Gather", "MPI_Allgather failed");
#else
	/* write my into gather */
	int len = my.Length();
	gather.CopyPart(Rank()*len, my, 0, len);
#endif

//NOTE: need to implement gather with sends and receives for CPLANT and other
//      platforms where Allgather seems to be troublesome.

	Log(kModerate, "AllGather", "gathered %d", gather.Length());
	if (LogLevel() == kLow) Log() << gather.wrap(4) << '\n';
}

/* gather multiple values from all */
void CommunicatorT::AllGather(const nArrayT<int>& my, nArrayT<int>& gather) const
{
	Log(kModerate, "AllGather", "sending %d", my.Length());
	if (LogLevel() == kLow) Log() << my.wrap(4) << '\n';

	/* check */
	if (my.Length()*Size() != gather.Length()) 
		ExceptionT::SizeMismatch("CommunicatorT::AllGather", 
			"buffer length %d does not match size %d", gather.Length(), my.Length()*Size());

#ifdef __TAHOE_MPI__
	int len = my.Length();
	if (MPI_Allgather(my.Pointer(), len, MPI_INT, gather.Pointer(), len, MPI_INT, fComm) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::Gather", "MPI_Allgather failed");
#else
	/* write my into gather */
	int len = my.Length();
	gather.CopyPart(Rank()*len, my, 0, len);
#endif

//NOTE: need to implement gather with sends and receives for CPLANT and other
//      platforms where Allgather seems to be troublesome.

	Log(kModerate, "AllGather", "gathered %d", gather.Length());
	if (LogLevel() == kLow) Log() << gather.wrap(4) << '\n';
}

/* gather multiple values from all */
void CommunicatorT::AllGather(const nArrayT<int>& counts, const nArrayT<int>& displacements, 
	const nArrayT<double>& my, nArrayT<double>& gather) const
{
	Log(kModerate, "AllGather", "sending %d", my.Length());
	if (LogLevel() == kLow)
	{
		Log() << "counts = \n" << counts.wrap(4) << '\n';
		Log() << "displacements = \n" << displacements.wrap(4) << '\n';
		Log() << "sending = \n" << my.wrap(4) << '\n';		
	}

#if __option(extended_errorcheck)
	if (counts[Rank()] != my.Length()) ExceptionT::SizeMismatch();
	if (counts.Length() != displacements.Length()) ExceptionT::SizeMismatch();
	if (gather.Length() > 1 && 
		(displacements.Last() + counts.Last() >= gather.Length())) ExceptionT::SizeMismatch();
		/* assume counts and displacements are OK interms of overlap and assume the
		 * displacements are monotonically increasing */
#endif

#ifdef __TAHOE_MPI__
	int len = my.Length();
	if (MPI_Allgatherv(my.Pointer(), len, MPI_DOUBLE, 
		gather.Pointer(), counts.Pointer(), displacements.Pointer(), MPI_DOUBLE, fComm) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::Gather", "MPI_Allgatherv failed");
#else
	/* write my into gather */
	int len = my.Length();
	int offset = displacements[Rank()];
	gather.CopyPart(offset, my, 0, len);
#endif

//NOTE: need to implement gather with sends and receives for CPLANT and other
//      platforms where Allgather seems to be troublesome.

	Log(kModerate, "AllGather", "gathered %d", gather.Length());
	if (LogLevel() == kLow) Log() << gather.wrap(4) << '\n';
}

/* gather multiple values from all */
void CommunicatorT::AllGather(const nArrayT<int>& counts, const nArrayT<int>& displacements, 
	const nArrayT<int>& my, nArrayT<int>& gather) const
{
	Log(kModerate, "AllGather", "sending %d", my.Length());
	if (LogLevel() == kLow)
	{
		Log() << "counts = \n" << counts.wrap(4) << '\n';
		Log() << "displacements = \n" << displacements.wrap(4) << '\n';
		Log() << "sending = \n" << my.wrap(4) << '\n';		
	}

#if __option(extended_errorcheck)
	if (counts[Rank()] != my.Length()) ExceptionT::SizeMismatch();
	if (counts.Length() != displacements.Length()) ExceptionT::SizeMismatch();
	if (gather.Length() > 1 && 
		(displacements.Last() + counts.Last() >= gather.Length())) ExceptionT::SizeMismatch();
		/* assume counts and displacements are OK interms of overlap and assume the
		 * displacements are monotonically increasing */
#endif

#ifdef __TAHOE_MPI__
	int len = my.Length();
	if (MPI_Allgatherv(my.Pointer(), len, MPI_INT, 
		gather.Pointer(), counts.Pointer(), displacements.Pointer(), MPI_INT, fComm) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::Gather", "MPI_Allgatherv failed");
#else
	/* write my into gather */
	int len = my.Length();
	int offset = displacements[Rank()];
	gather.CopyPart(offset, my, 0, len);
#endif

//NOTE: need to implement gather with sends and receives for CPLANT and other
//      platforms where Allgather seems to be troublesome.

	Log(kModerate, "AllGather", "gathered %d", gather.Length());
	if (LogLevel() == kLow) Log() << gather.wrap(4) << '\n';
}

/* broadcast character array */
void CommunicatorT::Broadcast(int source, ArrayT<char>& data)
{
	if (source == Rank())
	{
		Log(kModerate, "Broadcast", "sending %d", data.Length());
		if (LogLevel() == kLow) Log() << setw(10) << "data: " << data.Pointer() << '\n';
	}

#ifdef __TAHOE_MPI__
	if (MPI_Bcast(data.Pointer(), data.Length(), MPI_CHAR, source, fComm) != MPI_SUCCESS)
		Log(kFail, "CommunicatorT::Broadcast", "MPI_Bcast failed");
#endif

	if (source != Rank())
	{
		Log(kModerate, "Broadcast", "received %d", data.Length());
		if (LogLevel() == kLow) Log() << setw(10) << "data: " << data.Pointer() << '\n';
	}
}

/*************************************************************************
 * Private
 *************************************************************************/

void CommunicatorT::Init(void)
{
	/* environment was shut down */
	if (fCount == -1)
		ExceptionT::MPIFail("CommunicatorT::Init", "cannot restart MPI environment");

	/* communicator count */
	fCount++;

#ifdef __TAHOE_MPI__
	if (fCount == 1)
	{
		int *argc_ = fargc, argc = 1;
		char ***argv_ = fargv, **argv = NULL;

		/* need dummy arguments */
		if (!fargv) {
			argc_ = &argc;
			argv_ = &argv;
			argv = new char*[1];
			argv[0] = new char[2];
			argv[0][0] = 'a';
			argv[0][1] = '\0';
		}

		/* initialize MPI environment */
		if (MPI_Init(argc_, argv_) != MPI_SUCCESS)
			Log(kFail, "CommunicatorT::Init", "MPI_Init failed");
		
		/* free */
		if (!fargv) {
			for (int i = 0; i < argc; i++)
				delete[] argv[i];
			delete[] argv;
		}
	}
#endif	

	Log(kModerate, "Init", "communicator count = %d", fCount);
}

void CommunicatorT::Finalize(void)
{
	/* environment was shut down */
	if (fCount == -1)
		ExceptionT::MPIFail("CommunicatorT::Finalize", "MPI environment already down");

	/* communicator count */
	fCount--;

#ifdef __TAHOE_MPI__
	if (fCount == 0)
	{
		/* shut down MPI environment */
		if (MPI_Finalize() != MPI_SUCCESS)
			Log(kFail, "CommunicatorT::Finalize", "MPI_Finalized failed");
	}
#endif

	/* close */
	if (fCount == 0) fCount = -1;
	Log(kModerate, "Init", "communicator count = %d", fCount);
}

void CommunicatorT::doLog(const char* caller, const char* message) const
{
	ostream& out = *fLog;

	/* write info */
	time_t t;
	time(&t);
	out << '\n' 
	    << setw(10) << "CommunicatorT::doLog: rank: " << Rank() << '\n' 
	    << setw(10) << "time: " << ctime(&t);
	if (caller)
		out << setw(10) << "caller: " << caller << '\n';
	if (message)
		out << setw(10) << "message: " << message << '\n';
		
	/* flush stream */
	if (fLogLevel != kLow) out.flush();
}
