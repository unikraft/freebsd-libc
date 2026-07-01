/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal shim for FreeBSD's libc_private.h. Many FreeBSD sources include it
 * defensively for pthread/namespace/allocator hooks the port doesn't have, so
 * a near-empty shim satisfies the include; symbols are added on demand.
 */
#ifndef _LIBC_PRIVATE_H_
#define _LIBC_PRIVATE_H_

/*
 * gdtoa's lock macros read this before taking their scratch-list mutex; single-
 * threaded, so it stays 0 and the locks are never taken. Defined in glue/stubs.c.
 */
extern int __isthreaded;

/*
 * Program name. getopt.c/err(3)/warnx prefix diagnostics with _getprogname(),
 * which is the real FreeBSD gen/getprogname.c (returns __progname; setprogname
 * sets it). __progname has a fixed default ("unikraft") in glue/stubs.c.
 */
extern const char *__progname;
const char *_getprogname(void);

#if defined(CONFIG_LIBFREEBSD_LIBC_STDIO)
/*
 * FreeBSD stdio port: FILE-locking + cancellation-cleanup shims. Single-
 * threaded with no cancellation, so all of it is a no-op: FLOCKFILE/FUNLOCKFILE
 * collapse to nothing (the real bodies stay in _flock_stub.c for callers that
 * take the address), and the cancel push/pop drops the cleanup routine so it
 * never becomes a link dependency. Only present behind the stdio flag.
 */
#ifndef FLOCKFILE
#define FLOCKFILE(fp)		((void)0)
#define FUNLOCKFILE(fp)		((void)0)
#endif
#ifndef _FLOCKFILE
#define _FLOCKFILE(fp)		((void)0)
#define _FUNLOCKFILE(fp)	((void)0)
#endif

struct _pthread_cleanup_info {
	long	__pthread_cleanup_pad[8];
};

/*
 * __cleanup is the exit()/abort() hook to flush+close all stdio streams;
 * findfp.c sets it, and it's defined (NULL) in glue/stubs.c. The exit-path
 * wiring lives in the platform glue (else buffered stdout isn't flushed).
 */
extern void (*__cleanup)(void);

/* findfp.c's deprecated f_prealloc() sizes itself from the fd-table size. */
int getdtablesize(void);

#ifndef ___pthread_cleanup_push_imp
#define ___pthread_cleanup_push_imp(rtn, arg, info) \
	do { (void)(arg); (void)(info); } while (0)
#define ___pthread_cleanup_pop_imp(execute) \
	do { (void)(execute); } while (0)
#endif
#endif /* CONFIG_LIBFREEBSD_LIBC_STDIO */

#endif /* _LIBC_PRIVATE_H_ */
