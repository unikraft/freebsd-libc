/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <spinlock.h> shim. findfp.c includes it for the global FILE-list
 * lock; single-threaded here, so it's a genuine no-op. If threads are ever
 * added, replace with real Unikraft locks.
 */
#ifndef _SPINLOCK_FREEBSDPORT_SHIM_H_
#define _SPINLOCK_FREEBSDPORT_SHIM_H_

typedef int spinlock_t;

#define _SPINLOCK_INITIALIZER 0
#define _SPINLOCK(sl)		((void)(sl))
#define _SPINUNLOCK(sl)		((void)(sl))
#define _SPINTRYLOCK(sl)	(0)

/* findfp.c uses these names for the stdio internal list lock. */
#define STDIO_THREAD_LOCK()	((void)0)
#define STDIO_THREAD_UNLOCK()	((void)0)

#endif /* _SPINLOCK_FREEBSDPORT_SHIM_H_ */
