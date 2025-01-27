#include "time.h"
#ifndef USE_CLOCK
#include "sys/types.h"
#include "sys/times.h"
#endif

#undef Hz
#ifdef CLK_TCK
#define Hz CLK_TCK
#else
#ifdef HZ
#define Hz HZ
#else
#define Hz 60
#endif
#endif

 double
#ifdef KR_headers
etime_(tarray) float *tarray;
#else
etime_(float *tarray)
#endif
{
#ifdef USE_CLOCK
#ifndef CLOCKS_PER_SECOND
#define CLOCKS_PER_SECOND Hz
#endif
	double t = clock();
	tarray[1] = 0;
	return tarray[0] = t / CLOCKS_PER_SECOND;
#else
	struct tms t;

	times(&t);
	return (tarray[0] = t.tms_utime/Hz) + (tarray[1] = t.tms_stime/Hz);
#endif
	}
