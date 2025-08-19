/*
 * FreeBSD Clock Constants Wrapper
 * 
 * This header provides a compatibility layer for FreeBSD clock constants,
 * preventing conflicts with Unikraft's time implementation.
 * 
 * CRITICAL: Must be included INSTEAD of sys/_clock_id.h directly
 */

#ifndef _FREEBSD_CLOCK_WRAPPER_H_
#define _FREEBSD_CLOCK_WRAPPER_H_

/* Pre-define our clock constants to prevent FreeBSD from overriding them */
#define CLOCK_REALTIME           0
#define CLOCK_MONOTONIC          4   /* FreeBSD standard value */
#define CLOCK_PROCESS_CPUTIME_ID 15
#define CLOCK_THREAD_CPUTIME_ID  18  /* Moved to avoid conflicts */

/* CRITICAL: Give CLOCK_BOOTTIME a unique value to prevent duplicate case errors */
#define CLOCK_BOOTTIME           13  /* NOT an alias to CLOCK_MONOTONIC */
#define CLOCK_MONOTONIC_RAW      14  /* Missing in FreeBSD, needed by Unikraft */

/* FreeBSD-specific clocks */
#define CLOCK_MONOTONIC_PRECISE  11
#define CLOCK_MONOTONIC_FAST     12
#define CLOCK_MONOTONIC_COARSE   CLOCK_MONOTONIC_FAST

/* Prevent FreeBSD from redefining these by marking them as already defined */
#define _CLOCK_REALTIME_DEFINED
#define _CLOCK_MONOTONIC_DEFINED
#define _CLOCK_BOOTTIME_DEFINED
#define _CLOCK_MONOTONIC_RAW_DEFINED

/* Mark that we've handled clock definitions */
#define _FREEBSD_CLOCK_WRAPPER_INCLUDED 1

#endif /* _FREEBSD_CLOCK_WRAPPER_H_ */