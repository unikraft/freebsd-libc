/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal sys/resource.h for SQLite's shell.c. nolibc does not ship this.
 * The shell only uses getrusage() to print timings; we stub that out.
 */
#ifndef _SYS_RESOURCE_FREEBSDPORT_H_
#define _SYS_RESOURCE_FREEBSDPORT_H_

#include <sys/types.h>
#include <sys/time.h>

#define RUSAGE_SELF     0
#define RUSAGE_CHILDREN (-1)
#define RUSAGE_THREAD   1

struct rusage {
	struct timeval ru_utime;
	struct timeval ru_stime;
	long ru_maxrss;
	long ru_ixrss;
	long ru_idrss;
	long ru_isrss;
	long ru_minflt;
	long ru_majflt;
	long ru_nswap;
	long ru_inblock;
	long ru_oublock;
	long ru_msgsnd;
	long ru_msgrcv;
	long ru_nsignals;
	long ru_nvcsw;
	long ru_nivcsw;
};

struct rlimit {
	unsigned long long rlim_cur;
	unsigned long long rlim_max;
};

#define RLIMIT_CPU     0
#define RLIMIT_FSIZE   1
#define RLIMIT_DATA    2
#define RLIMIT_STACK   3
#define RLIMIT_CORE    4
#define RLIMIT_RSS     5
#define RLIMIT_NOFILE  7
#define RLIMIT_AS      9
#define RLIM_INFINITY  ((unsigned long long)-1)

static inline int getrusage(int who __attribute__((unused)),
			     struct rusage *ru)
{
	if (ru) {
		ru->ru_utime.tv_sec = 0;
		ru->ru_utime.tv_usec = 0;
		ru->ru_stime.tv_sec = 0;
		ru->ru_stime.tv_usec = 0;
		ru->ru_maxrss = 0;
		ru->ru_minflt = 0;
		ru->ru_majflt = 0;
	}
	return 0;
}

static inline int getrlimit(int resource __attribute__((unused)),
			     struct rlimit *rlim)
{
	if (rlim) {
		rlim->rlim_cur = RLIM_INFINITY;
		rlim->rlim_max = RLIM_INFINITY;
	}
	return 0;
}

static inline int setrlimit(int resource __attribute__((unused)),
			     const struct rlimit *rlim __attribute__((unused)))
{
	return 0;
}

#endif /* _SYS_RESOURCE_FREEBSDPORT_H_ */
