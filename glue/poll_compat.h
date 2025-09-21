/*
 * FreeBSD libc Poll/Epoll Compatibility Layer
 * 
 * This header ensures FreeBSD libc poll/epoll constants are compatible with
 * Unikraft's expectations. Unikraft assumes Linux-style values.
 * 
 * References:
 * - FreeBSD: sys/poll.h, sys/epoll.h  
 * - Linux: sys/epoll.h
 * - Unikraft: lib/posix-poll/poll.c (requires constants to match)
 */

#ifndef _FREEBSD_POLL_COMPAT_H_
#define _FREEBSD_POLL_COMPAT_H_

#include <sys/poll.h>

/*
 * CRITICAL: Unikraft requires poll/epoll constants to match exactly
 * FreeBSD has different values than Linux for some constants.
 * We need to override FreeBSD values to match Unikraft's expectations.
 */

/* Document the original FreeBSD values for reference */
#ifdef EPOLLRDHUP
#define FREEBSD_EPOLLRDHUP EPOLLRDHUP    /* 0x2000 */
#undef EPOLLRDHUP
#endif

#ifdef EPOLLWRNORM  
#define FREEBSD_EPOLLWRNORM EPOLLWRNORM  /* 0x100 */
#undef EPOLLWRNORM
#endif

#ifdef EPOLLWRBAND
#define FREEBSD_EPOLLWRBAND EPOLLWRBAND  /* 0x200 */  
#undef EPOLLWRBAND
#endif

#ifdef POLLRDHUP
#define FREEBSD_POLLRDHUP POLLRDHUP      /* 0x4000 */
#undef POLLRDHUP
#endif

#ifdef POLLWRNORM
#define FREEBSD_POLLWRNORM POLLWRNORM    /* POLLOUT alias */
#undef POLLWRNORM
#endif

#ifdef POLLWRBAND  
#define FREEBSD_POLLWRBAND POLLWRBAND    /* 0x0100 */
#undef POLLWRBAND
#endif

/* 
 * Define Linux-compatible values that Unikraft expects
 * These values are from Linux sys/epoll.h
 */
#define EPOLLRDHUP  0x2000    /* Stream socket peer closed connection */
#define EPOLLWRNORM 0x100     /* Normal data may be written */
#define EPOLLWRBAND 0x200     /* Priority data may be written */

#define POLLRDHUP   0x2000    /* Stream socket peer closed connection - match epoll */
#define POLLWRNORM  0x100     /* Normal data may be written - match epoll */  
#define POLLWRBAND  0x200     /* Priority data may be written - match epoll */

/* 
 * Ensure missing constants are defined if needed
 */
#ifndef EPOLLRDNORM
#define EPOLLRDNORM 0x040     /* Normal data may be read */
#endif

#ifndef EPOLLRDBAND  
#define EPOLLRDBAND 0x080     /* Priority data may be read */
#endif

#ifndef POLLRDNORM
#define POLLRDNORM  0x040     /* Normal data may be read - match epoll */
#endif

#ifndef POLLRDBAND
#define POLLRDBAND  0x080     /* Priority data may be read - match epoll */
#endif

/*
 * Runtime warning for debugging - these will show which constants were remapped
 */
#pragma message "FreeBSD libc: Remapped poll/epoll constants for Unikraft compatibility"
#pragma message "POLLRDHUP: FreeBSD=0x4000 -> Linux=0x2000"
#pragma message "POLLWRNORM: FreeBSD=POLLOUT -> Linux=0x100"  
#pragma message "POLLWRBAND: FreeBSD=0x0100 -> Linux=0x200"

#endif /* _FREEBSD_POLL_COMPAT_H_ */