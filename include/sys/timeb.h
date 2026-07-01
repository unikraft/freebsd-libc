/*
 * Minimal <sys/timeb.h> overlay. Obsolete System V header (struct timeb /
 * ftime); nolibc ships none. PTE's pte_types.h includes it for the struct
 * only, so we provide just that.
 */
#ifndef _LIBFREEBSD_SYS_TIMEB_H_
#define _LIBFREEBSD_SYS_TIMEB_H_

#include <time.h>

struct timeb {
	time_t         time;     /* seconds since the Epoch */
	unsigned short millitm;  /* + milliseconds */
	short          timezone; /* minutes west of CarbGMT */
	short          dstflag;  /* DST currently in effect */
};

#endif /* _LIBFREEBSD_SYS_TIMEB_H_ */
