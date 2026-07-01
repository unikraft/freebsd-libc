/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal pthread shim for the single-threaded unikernel. gdtoa (and stdio)
 * take libc-internal mutexes through _pthread_mutex_lock/unlock; single-
 * threaded, so they're no-ops. Provides just enough of <pthread.h> to compile
 * those sources unpatched. Not real threading - for that, enable
 * CONFIG_LIBPTHREAD_EMBEDDED (handled below).
 */
#ifndef _PTHREAD_H_
#define _PTHREAD_H_

/*
 * Pull in the Kconfig macros ourselves: a TU that includes <pthread.h> before
 * <uk/config.h> would otherwise see CONFIG_LIBPTHREAD_EMBEDDED undefined and
 * wrongly take the single-threaded branch below.
 */
#include <uk/config.h>

#if defined(CONFIG_LIBPTHREAD_EMBEDDED)
/*
 * Real-threads mode. With PTE enabled, defer the standard pthread API/types to
 * PTE's real <pthread.h> (ours -> PTE -> real, via the -I order). PTE doesn't
 * provide FreeBSD's internal underscore lock helpers, which gdtoa/stdio use
 * under `if (__isthreaded)`; __isthreaded == 0, so they're never taken and the
 * no-ops below only need to exist to compile. App-level threading is fully real.
 */
#ifdef __cplusplus
extern "C" {
#endif
#include_next <pthread.h>

static inline int _pthread_mutex_lock(void *m)    { (void)m; return 0; }
static inline int _pthread_mutex_unlock(void *m)  { (void)m; return 0; }
static inline int _pthread_mutex_trylock(void *m) { (void)m; return 0; }
static inline pthread_t _pthread_self(void)       { return pthread_self(); }

#ifdef __cplusplus
}
#endif

#else /* !CONFIG_LIBPTHREAD_EMBEDDED - single-threaded shim (default) */

#ifdef __cplusplus
extern "C" {
#endif

typedef int pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER 0

/*
 * The helpers take void* so the same no-ops serve gdtoa's pthread_mutex_t[]
 * (int) and stdio's _flock_stub.c (which passes a struct pthread_mutex **).
 */
static inline int _pthread_mutex_lock(void *m)     { (void)m; return 0; }
static inline int _pthread_mutex_unlock(void *m)   { (void)m; return 0; }
static inline int _pthread_mutex_trylock(void *m)  { (void)m; return 0; }
static inline int pthread_mutex_lock(void *m)      { (void)m; return 0; }
static inline int pthread_mutex_unlock(void *m)    { (void)m; return 0; }

/*
 * _flock_stub.c compares FILE's _fl_owner against _pthread_self(); single-
 * threaded, so a constant NULL identity is consistent (never contended).
 */
/*
 * Match nolibc's guarded pthread_t (struct __pthread *) with the same tag and
 * __DEFINED_pthread_t guard, so the two headers agree regardless of include
 * order (a different tag caused a "conflicting types" error under OpenSSL).
 *
 * Old line (kept for reference):
 * typedef struct pthread *pthread_t;
 */
#ifndef __DEFINED_pthread_t
typedef struct __pthread *pthread_t;
#define __DEFINED_pthread_t
#endif
static inline pthread_t _pthread_self(void) { return (pthread_t)0; }
static inline pthread_t pthread_self(void)  { return (pthread_t)0; }
static inline int pthread_equal(pthread_t a, pthread_t b) { return a == b; }

/*
 * Single-threaded pthread surface for OpenSSL (crypto/threads_pthread.c). All
 * locking is uncontended, so mutex/rwlock/once operate on caller-owned state
 * with no atomics. Only TLS needs a shared backing table (glue/pthread_shim.c).
 * Correct only because nothing here runs concurrently.
 */

/* --- run-once: the flag is caller-owned, so this is purely local --- */
typedef int pthread_once_t;
#define PTHREAD_ONCE_INIT 0
static inline int pthread_once(pthread_once_t *o, void (*init)(void))
{
	if (*o == 0) { *o = 1; init(); }
	return 0;
}

/* --- mutex (+ attributes): uncontended no-ops --- */
typedef int pthread_mutexattr_t;
#define PTHREAD_MUTEX_RECURSIVE 1
static inline int pthread_mutexattr_init(pthread_mutexattr_t *a)            { (void)a; return 0; }
static inline int pthread_mutexattr_destroy(pthread_mutexattr_t *a)         { (void)a; return 0; }
static inline int pthread_mutexattr_settype(pthread_mutexattr_t *a, int t)  { (void)a; (void)t; return 0; }
static inline int pthread_mutex_init(void *m, const void *a)                { (void)m; (void)a; return 0; }
static inline int pthread_mutex_destroy(void *m)                            { (void)m; return 0; }

/* --- rwlock: uncontended no-ops --- */
typedef int pthread_rwlock_t;
#define PTHREAD_RWLOCK_INITIALIZER 0
static inline int pthread_rwlock_init(pthread_rwlock_t *l, const void *a)   { (void)l; (void)a; return 0; }
static inline int pthread_rwlock_destroy(pthread_rwlock_t *l)               { (void)l; return 0; }
static inline int pthread_rwlock_rdlock(pthread_rwlock_t *l)                { (void)l; return 0; }
static inline int pthread_rwlock_wrlock(pthread_rwlock_t *l)                { (void)l; return 0; }
static inline int pthread_rwlock_unlock(pthread_rwlock_t *l)                { (void)l; return 0; }

/* --- no process model: fork handlers never fire --- */
static inline int pthread_atfork(void (*prepare)(void), void (*parent)(void),
				 void (*child)(void))
{
	(void)prepare; (void)parent; (void)child; return 0;
}

/* --- thread-local storage: shared table in glue/pthread_shim.c --- */
typedef unsigned int pthread_key_t;
int pthread_key_create(pthread_key_t *key, void (*destructor)(void *));
int pthread_key_delete(pthread_key_t key);
void *pthread_getspecific(pthread_key_t key);
int pthread_setspecific(pthread_key_t key, const void *value);

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_LIBPTHREAD_EMBEDDED */

#endif /* _PTHREAD_H_ */
