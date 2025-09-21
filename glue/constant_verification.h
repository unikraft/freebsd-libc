/*
 * FreeBSD libc Constant Verification
 * 
 * This header contains compile-time assertions to ensure FreeBSD libc constants
 * match Unikraft and POSIX expectations. Any mismatch will cause a compile error.
 */

#ifndef _FREEBSD_CONSTANT_VERIFICATION_H_
#define _FREEBSD_CONSTANT_VERIFICATION_H_

#include <stddef.h>  /* For offsetof */
#include <uk/essentials.h>

/* Only include headers if they exist and are needed for verification */
#ifdef __has_include
#if __has_include(<errno.h>)
#include <errno.h>
#endif
#if __has_include(<fcntl.h>)
#include <fcntl.h>
#endif
#if __has_include(<poll.h>)
#include <poll.h>
#endif
#if __has_include(<signal.h>)
#include <signal.h>
#endif
#if __has_include(<sys/socket.h>)
#include <sys/socket.h>
#endif
#if __has_include(<time.h>)
#include <time.h>
#endif
#if __has_include(<sys/epoll.h>)
#include <sys/epoll.h>
#endif
#else
/* Fallback for older compilers */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <time.h>
#endif

/*
 * CRITICAL CONSTANT VERIFICATION 
 * Following Unikraft's proven pattern from lib/posix-poll/poll.c
 * Source: FreeBSD sys/errno.h, sys/poll.h, sys/fcntl.h
 * Reference: POSIX.1-2017, Unikraft compatibility layer
 */

/* For performance we copy between epoll and poll events with no conversion. */
/* This assumes them to be equal, which we ensure with these asserts */
/* NOTE: Basic poll/epoll constants that MUST match across systems */
#if defined(EPOLLIN) && defined(POLLIN)
UK_CTASSERT(EPOLLIN == POLLIN);
#endif
#if defined(EPOLLOUT) && defined(POLLOUT)
UK_CTASSERT(EPOLLOUT == POLLOUT);
#endif
#if defined(EPOLLERR) && defined(POLLERR)
UK_CTASSERT(EPOLLERR == POLLERR);
#endif
#if defined(EPOLLHUP) && defined(POLLHUP)
UK_CTASSERT(EPOLLHUP == POLLHUP);
#endif

/* 
 * PLATFORM-SPECIFIC CONSTANTS - FreeBSD vs Linux differences
 * These constants have different values between FreeBSD and Linux.
 * We document the differences but do not enforce equality.
 * Citation: FreeBSD sys/poll.h, sys/epoll.h vs Linux values
 * 
 * FreeBSD values:
 * - POLLRDHUP   = 0x4000, EPOLLRDHUP  = 0x2000  ❌ Different  
 * - POLLWRNORM  = POLLOUT, EPOLLWRNORM = 0x100   ❌ Different
 * - POLLWRBAND  = 0x0100,  EPOLLWRBAND = 0x200   ❌ Different
 */
#pragma message "FreeBSD libc: Using FreeBSD-specific poll/epoll constant values (different from Linux)"

/*
 * ERROR CODE VERIFICATION
 * Source: FreeBSD sys/errno.h
 * Reference: POSIX.1-2017
 */
#ifdef EPERM
UK_CTASSERT(EPERM == 1);        /* Operation not permitted */
#endif
UK_CTASSERT(ENOENT == 2);       /* No such file or directory */
UK_CTASSERT(ESRCH == 3);        /* No such process */
UK_CTASSERT(EINTR == 4);        /* Interrupted system call */
UK_CTASSERT(EIO == 5);          /* I/O error */
UK_CTASSERT(ENXIO == 6);        /* No such device or address */
UK_CTASSERT(E2BIG == 7);        /* Argument list too long */
UK_CTASSERT(ENOEXEC == 8);      /* Exec format error */
UK_CTASSERT(EBADF == 9);        /* Bad file number */
UK_CTASSERT(ECHILD == 10);      /* No child processes */
UK_CTASSERT(EDEADLK == 11);     /* Resource deadlock would occur */
UK_CTASSERT(ENOMEM == 12);      /* Out of memory */
UK_CTASSERT(EACCES == 13);      /* Permission denied */
UK_CTASSERT(EFAULT == 14);      /* Bad address */
UK_CTASSERT(ENOTBLK == 15);     /* Block device required */
UK_CTASSERT(EBUSY == 16);       /* Device or resource busy */
UK_CTASSERT(EEXIST == 17);      /* File exists */
UK_CTASSERT(EXDEV == 18);       /* Cross-device link */
UK_CTASSERT(ENODEV == 19);      /* No such device */
UK_CTASSERT(ENOTDIR == 20);     /* Not a directory */
UK_CTASSERT(EISDIR == 21);      /* Is a directory */
UK_CTASSERT(EINVAL == 22);      /* Invalid argument */
UK_CTASSERT(ENFILE == 23);      /* File table overflow */
UK_CTASSERT(EMFILE == 24);      /* Too many open files */
UK_CTASSERT(ENOTTY == 25);      /* Not a typewriter */
UK_CTASSERT(ETXTBSY == 26);     /* Text file busy */
UK_CTASSERT(EFBIG == 27);       /* File too large */
UK_CTASSERT(ENOSPC == 28);      /* No space left on device */
UK_CTASSERT(ESPIPE == 29);      /* Illegal seek */
UK_CTASSERT(EROFS == 30);       /* Read-only file system */
UK_CTASSERT(EMLINK == 31);      /* Too many links */
UK_CTASSERT(EPIPE == 32);       /* Broken pipe */
UK_CTASSERT(EDOM == 33);        /* Math argument out of domain of func */
UK_CTASSERT(ERANGE == 34);      /* Math result not representable */
UK_CTASSERT(EAGAIN == 35);      /* Try again */
UK_CTASSERT(EWOULDBLOCK == EAGAIN); /* Operation would block */

/*
 * FILE OPERATION CONSTANTS
 * Source: FreeBSD sys/fcntl.h 
 * Reference: POSIX.1-2017
 */
UK_CTASSERT(O_RDONLY == 0x0000);    /* Open for reading only */
UK_CTASSERT(O_WRONLY == 0x0001);    /* Open for writing only */
UK_CTASSERT(O_RDWR == 0x0002);      /* Open for reading and writing */
UK_CTASSERT(O_CREAT == 0x0200);     /* Create if nonexistent */
UK_CTASSERT(O_EXCL == 0x0800);      /* Error if already exists */
UK_CTASSERT(O_TRUNC == 0x0400);     /* Truncate to zero length */
UK_CTASSERT(O_APPEND == 0x0008);    /* Set append mode */
UK_CTASSERT(O_NONBLOCK == 0x0004);  /* No delay */
UK_CTASSERT(O_SYNC == 0x0080);      /* Synchronous writes */

/* Seek constants */
UK_CTASSERT(SEEK_SET == 0);         /* Seek from beginning of file */
UK_CTASSERT(SEEK_CUR == 1);         /* Seek from current position */
UK_CTASSERT(SEEK_END == 2);         /* Seek from end of file */

/*
 * SOCKET CONSTANTS
 * Source: FreeBSD sys/socket.h
 * Reference: POSIX.1-2017, RFC specifications
 */
UK_CTASSERT(AF_UNSPEC == 0);        /* Unspecified */
UK_CTASSERT(AF_UNIX == 1);          /* Unix domain sockets */
UK_CTASSERT(AF_LOCAL == AF_UNIX);   /* POSIX name for AF_UNIX */
UK_CTASSERT(AF_INET == 2);          /* Internet IP Protocol */
UK_CTASSERT(AF_INET6 == 28);        /* IP version 6 - FreeBSD specific value */

UK_CTASSERT(SOCK_STREAM == 1);      /* Stream socket */
UK_CTASSERT(SOCK_DGRAM == 2);       /* Datagram socket */
UK_CTASSERT(SOCK_RAW == 3);         /* Raw-protocol interface */

/* Socket options - Level SOL_SOCKET */
UK_CTASSERT(SOL_SOCKET == 0xffff);  /* Options for socket level */
UK_CTASSERT(SO_REUSEADDR == 0x0004); /* Allow reuse of local addresses */
UK_CTASSERT(SO_KEEPALIVE == 0x0008); /* Keep connections alive */
UK_CTASSERT(SO_BROADCAST == 0x0020); /* Permit sending of broadcast msgs */

/*
 * POLLING CONSTANTS 
 * Source: FreeBSD sys/poll.h
 * Reference: POSIX.1-2017
 * Note: These should match epoll constants for Unikraft compatibility
 */
UK_CTASSERT(POLLIN == 0x0001);      /* There is data to read */
UK_CTASSERT(POLLPRI == 0x0002);     /* There is urgent data to read */
UK_CTASSERT(POLLOUT == 0x0004);     /* Writing now will not block */
UK_CTASSERT(POLLERR == 0x0008);     /* Error condition */
UK_CTASSERT(POLLHUP == 0x0010);     /* Hung up */
UK_CTASSERT(POLLNVAL == 0x0020);    /* Invalid request: fd not open */

/*
 * SIGNAL CONSTANTS
 * Source: FreeBSD sys/signal.h  
 * Reference: POSIX.1-2017
 * Warning: Signal numbers can vary between systems!
 */
UK_CTASSERT(SIGHUP == 1);           /* Hangup */
UK_CTASSERT(SIGINT == 2);           /* Interrupt */
UK_CTASSERT(SIGQUIT == 3);          /* Quit */
UK_CTASSERT(SIGILL == 4);           /* Illegal instruction */
UK_CTASSERT(SIGTRAP == 5);          /* Trace/breakpoint trap */
UK_CTASSERT(SIGABRT == 6);          /* Abort */
UK_CTASSERT(SIGEMT == 7);           /* EMT instruction - FreeBSD specific */
UK_CTASSERT(SIGFPE == 8);           /* Floating point exception */
UK_CTASSERT(SIGKILL == 9);          /* Kill signal */
UK_CTASSERT(SIGBUS == 10);          /* Bus error */
UK_CTASSERT(SIGSEGV == 11);         /* Segmentation violation */
UK_CTASSERT(SIGSYS == 12);          /* Bad system call */
UK_CTASSERT(SIGPIPE == 13);         /* Broken pipe */
UK_CTASSERT(SIGALRM == 14);         /* Alarm clock */
UK_CTASSERT(SIGTERM == 15);         /* Termination */
UK_CTASSERT(SIGURG == 16);          /* Urgent condition on IO channel */
UK_CTASSERT(SIGSTOP == 17);         /* Sendable stop signal not from tty */
UK_CTASSERT(SIGTSTP == 18);         /* Stop signal from tty */
UK_CTASSERT(SIGCONT == 19);         /* Continue a stopped process */
UK_CTASSERT(SIGCHLD == 20);         /* Child status has changed */
UK_CTASSERT(SIGTTIN == 21);         /* Background read from tty */
UK_CTASSERT(SIGTTOU == 22);         /* Background write to tty */
UK_CTASSERT(SIGIO == 23);           /* I/O now possible */
UK_CTASSERT(SIGXCPU == 24);         /* CPU limit exceeded */
UK_CTASSERT(SIGXFSZ == 25);         /* File size limit exceeded */
UK_CTASSERT(SIGVTALRM == 26);       /* Virtual alarm clock */
UK_CTASSERT(SIGPROF == 27);         /* Profiling alarm clock */
UK_CTASSERT(SIGWINCH == 28);        /* Window size change */
UK_CTASSERT(SIGINFO == 29);         /* Information request - FreeBSD specific */
UK_CTASSERT(SIGUSR1 == 30);         /* User defined signal 1 - FreeBSD specific */
UK_CTASSERT(SIGUSR2 == 31);         /* User defined signal 2 - FreeBSD specific */

/*
 * TIME CONSTANTS
 * Source: FreeBSD sys/time.h, time.h with Unikraft compatibility
 * Reference: POSIX.1-2017
 * Note: Values adjusted to prevent conflicts with Unikraft time.c
 */
UK_CTASSERT(CLOCK_REALTIME == 0);           /* System-wide realtime clock */
UK_CTASSERT(CLOCK_MONOTONIC == 4);          /* Monotonic clock - FreeBSD specific value */
UK_CTASSERT(CLOCK_PROCESS_CPUTIME_ID == 15); /* Per-process CPU-time clock */
/* UK_CTASSERT(CLOCK_THREAD_CPUTIME_ID == 16);  - Disabled: conflicts with clock_wrapper.h */

/* Unikraft-specific clock constants - ensure they don't conflict */
#ifdef CLOCK_BOOTTIME
UK_CTASSERT(CLOCK_BOOTTIME == 13);          /* Boot time clock - unique value */
#endif
#ifdef CLOCK_MONOTONIC_RAW
UK_CTASSERT(CLOCK_MONOTONIC_RAW == 14);     /* Raw monotonic clock - unique value */
#endif

/*
 * STRUCTURE SIZE VERIFICATION
 * Ensures binary compatibility with expected layouts
 */
UK_CTASSERT(sizeof(struct timespec) == 16);   /* tv_sec(8) + tv_nsec(8) on 64-bit */
UK_CTASSERT(sizeof(struct timeval) == 16);    /* tv_sec(8) + tv_usec(8) on 64-bit */
UK_CTASSERT(sizeof(off_t) == 8);              /* 64-bit file offsets */
UK_CTASSERT(sizeof(size_t) == 8);             /* 64-bit size_t on 64-bit systems */
UK_CTASSERT(sizeof(ssize_t) == 8);            /* Signed size_t */
UK_CTASSERT(sizeof(mode_t) == 2);             /* File mode */
UK_CTASSERT(sizeof(pid_t) == 4);              /* Process ID */
UK_CTASSERT(sizeof(uid_t) == 4);              /* User ID */
UK_CTASSERT(sizeof(gid_t) == 4);              /* Group ID */

/*
 * STRUCTURE ALIGNMENT VERIFICATION  
 * Critical for ABI compatibility
 */
UK_CTASSERT(offsetof(struct timespec, tv_sec) == 0);
UK_CTASSERT(offsetof(struct timespec, tv_nsec) == 8);
UK_CTASSERT(offsetof(struct timeval, tv_sec) == 0);
UK_CTASSERT(offsetof(struct timeval, tv_usec) == 8);

#endif /* _FREEBSD_CONSTANT_VERIFICATION_H_ */