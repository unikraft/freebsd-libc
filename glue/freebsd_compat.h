/* SPDX-License-Identifier: BSD-3-Clause */
/*
 *
 * Copyright (c) 2025, Unikraft GmbH and The Unikraft Authors.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * FreeBSD libc Compatibility Layer for Unikraft
 *
 * This header is the central component of the compatibility layer between
 * FreeBSD libc and the Unikraft kernel. Its primary purpose is to resolve
 * conflicts and inconsistencies in type definitions, macros, and constants
 * that arise when integrating a foreign C library with a unikernel.
 *
 * CRITICAL: This file must be force-included by the compiler (using -include)
 * BEFORE any other headers, including FreeBSD's own. This ensures that our
 * definitions take precedence and prevent redefinition errors.
 *
 * The key responsibilities of this file include:
 *  - Establishing a consistent set of compiler attribute macros (e.g.,
 *    __unused, __packed) to avoid conflicts between Unikraft and FreeBSD.
 *  - Defining a fixed set of architecture-specific macros (e.g., __WORDSIZE)
 *    to prevent FreeBSD's multi-architecture headers from making incorrect
 *    assumptions.
 *  - Resolving conflicts in system limits and constants (e.g., __OFF_MAX,
 *    CLOCK_BOOTTIME) that differ between FreeBSD and Unikraft.
 *  - Providing a complete, compatible definition of the FreeBSD FILE
 *    structure to enable stdio functionality.
 *  - Wrapping or redirecting problematic function calls (e.g., mount).
 */

#ifndef _FREEBSD_COMPAT_H_
#define _FREEBSD_COMPAT_H_

/*******************************************************************************
 * SECTION: Essential FreeBSD Macros
 *
 * This section ensures that fundamental FreeBSD macros (from <sys/cdefs.h>)
 * are defined first, and then prevents them from being redefined. It also
 * pre-includes Unikraft's essentials to establish a baseline.
 ******************************************************************************/

/* CRITICAL: First, let FreeBSD's sys/cdefs.h define essential macros */
#include <sys/cdefs.h>

/* Now prevent further inclusion and override problematic definitions */
#define _SYS_CDEFS_H_UNIKRAFT_COMPAT_INCLUDED 1

/* CRITICAL: Mark these macros as already defined to prevent sys/cdefs.h from redefining them */
#ifdef __unused
#define __UNIKRAFT_UNUSED_DEFINED 1
#endif

#ifdef __used
#define __UNIKRAFT_USED_DEFINED 1
#endif

#ifdef __packed
#define __UNIKRAFT_PACKED_DEFINED 1
#endif

#ifdef __section
#define __UNIKRAFT_SECTION_DEFINED 1
#endif

#ifdef __noinline
#define __UNIKRAFT_NOINLINE_DEFINED 1
#endif

#ifdef __always_inline
#define __UNIKRAFT_ALWAYS_INLINE_DEFINED 1
#endif

#ifdef __aligned
#define __UNIKRAFT_ALIGNED_DEFINED 1
#endif

#ifdef __deprecated
#define __UNIKRAFT_DEPRECATED_DEFINED 1
#endif

#ifdef __weak
#define __UNIKRAFT_WEAK_DEFINED 1
#endif

/* Pre-include Unikraft's essentials to establish precedence */
#include <uk/essentials.h>

/* CRITICAL: Include clock wrapper BEFORE any time-related headers */
#include "clock_wrapper.h"

/*******************************************************************************
 * SECTION: System Call and Feature Compatibility
 *
 * This section handles compatibility for system calls and features that differ
 * between FreeBSD and Unikraft, such as poll/epoll and stack smashing
 * protection.
 ******************************************************************************/

/* Disable FreeBSD SSP (Stack Smashing Protection) to prevent function conflicts */
#ifndef __SSP_FORTIFY_LEVEL
#define __SSP_FORTIFY_LEVEL 0
#endif

/* Disable _FORTIFY_SOURCE to prevent conflicts with actual implementations */
#ifdef _FORTIFY_SOURCE
#undef _FORTIFY_SOURCE
#endif
#define _FORTIFY_SOURCE 0

/* Include poll compatibility BEFORE any poll.h can be included */
#include "poll_compat.h"

/* Now prevent FreeBSD from redefining these - define them to their current values */
#ifndef __unused
#define __unused __attribute__((unused))
#endif

#ifndef __used
#define __used __attribute__((used))
#endif

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#ifndef __aligned
#define __aligned(x) __attribute__((aligned(x)))
#endif

#ifndef __section
#define __section(s) __attribute__((section(s)))
#endif

#ifndef __noinline
#define __noinline __attribute__((noinline))
#endif

#ifndef __always_inline
#define __always_inline __attribute__((always_inline))
#endif

#ifndef __deprecated
#define __deprecated __attribute__((deprecated))
#endif

#ifndef __weak
#define __weak __attribute__((weak))
#endif

/*******************************************************************************
 * SECTION: Architecture and Type Definitions
 *
 * This section establishes a consistent set of architecture-specific
 * definitions and type limits, ensuring that both Unikraft and FreeBSD code
 * agree on fundamental properties like word size and file offsets.
 ******************************************************************************/

#ifndef __WORDSIZE
#define __WORDSIZE 64   /* Unikraft runs on x86_64 only - 64-bit words */
#endif

/* Prevent __OFF_MAX redefinition by pre-defining with consistent values */
#ifndef __OFF_MAX
#define __OFF_MAX 9223372036854775807LL  /* Maximum file offset for 64-bit systems */
#endif

#ifndef __OFF_MIN
#define __OFF_MIN (-__OFF_MAX - 1LL)  /* Minimum file offset for 64-bit systems */
#endif

/*******************************************************************************
 * SECTION: Clock Constants Compatibility
 *
 * Resolves a critical conflict between FreeBSD and Unikraft regarding clock
 * constants. FreeBSD defines CLOCK_BOOTTIME as an alias for CLOCK_MONOTONIC,
 * which causes "duplicate case value" errors in Unikraft's time handling.
 ******************************************************************************/

/* Prevent FreeBSD time headers from redefining clock constants */
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 0
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 4       /* Use FreeBSD's value but prevent redefinition */
#endif

#ifndef CLOCK_PROCESS_CPUTIME_ID
#define CLOCK_PROCESS_CPUTIME_ID 15
#endif

#ifndef CLOCK_THREAD_CPUTIME_ID
#define CLOCK_THREAD_CPUTIME_ID 16
#endif

/* CRITICAL: Prevent CLOCK_BOOTTIME from being defined as alias to CLOCK_MONOTONIC */
#ifndef CLOCK_BOOTTIME
#define CLOCK_BOOTTIME 13       /* Unique value, not alias to CLOCK_MONOTONIC */
#endif

/* Define missing CLOCK_MONOTONIC_RAW to prevent undeclared identifier error */
#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW 14  /* Unique value for raw monotonic clock */
#endif

/* FreeBSD-specific fast clocks - ensure they have unique values */
#ifndef CLOCK_MONOTONIC_PRECISE
#define CLOCK_MONOTONIC_PRECISE 11
#endif

#ifndef CLOCK_MONOTONIC_FAST
#define CLOCK_MONOTONIC_FAST 12
#endif

#ifndef CLOCK_MONOTONIC_COARSE
#define CLOCK_MONOTONIC_COARSE CLOCK_MONOTONIC_FAST  /* Alias to fast version */
#endif

/* Prevent cache line size conflicts - use consistent definition */
#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE 64  /* x86_64 cache line size */
#endif

/*******************************************************************************
 * SECTION: System and Compiler Helper Macros
 *
 * This section ensures that common system limits and compiler helper macros
 * (e.g., offsetof, containerof) are defined consistently, preventing conflicts.
 ******************************************************************************/

#ifndef __LONG_LONG_MAX
#define __LONG_LONG_MAX 9223372036854775807LL
#endif

/* Prevent UINTPTR_MAX redefinition conflicts */
#ifdef __UINTPTR_MAX
#undef __UINTPTR_MAX
#endif
#define __UINTPTR_MAX 0xffffffffffffffffUL

#ifndef __SIZE_MAX
#define __SIZE_MAX __UINTPTR_MAX
#endif

/* Prevent offsetof redefinition conflicts */
#ifdef __offsetof
#undef __offsetof
#endif
#define __offsetof(type, field)  __builtin_offsetof(type, field)

/* Prevent containerof redefinition conflicts */
#ifdef __containerof
#undef __containerof
#endif
#define __containerof(x, s, m) \
    ((s *)((char *)(x) - __offsetof(s, m)))

/*******************************************************************************
 * SECTION: Platform and Type Compatibility
 *
 * This section forces FreeBSD headers to use 64-bit definitions and ensures
 * that fundamental types are defined consistently.
 ******************************************************************************/

/* Force FreeBSD headers to use 64-bit definitions */
#ifndef __LP64__
#define __LP64__ 1
#endif

/* Prevent 32-bit assumptions */
#ifdef __ILP32__
#undef __ILP32__
#endif

/* Handle rlim_t type conflicts between FreeBSD and musl */
#ifdef _RLIM_T_DECLARED
/* FreeBSD has already declared rlim_t, prevent musl from redefining it */
#define RLIM_T_ALREADY_DEFINED 1
#endif

#ifndef __SIZEOF_POINTER__
#define __SIZEOF_POINTER__ 8  /* 64-bit pointers */
#endif

#ifndef __SIZEOF_LONG__
#define __SIZEOF_LONG__ 8     /* 64-bit longs on x86_64 */
#endif

#ifndef __SIZEOF_SIZE_T__
#define __SIZEOF_SIZE_T__ 8   /* 64-bit size_t */
#endif

/*******************************************************************************
 * SECTION: FreeBSD FILE Structure Integration
 *
 * This section provides a complete, compatible definition of the FreeBSD FILE
 * structure. This is essential for enabling stdio functionality.
 ******************************************************************************/

#include <sys/types.h>
#include <stdarg.h>

/* Forward declarations for dependencies - only if not already defined by FreeBSD headers */
#ifndef _FPOS_T_DECLARED
typedef long fpos_t;
#define _FPOS_T_DECLARED
#endif

/* Don't declare pthread types if FreeBSD headers already included them */
#ifndef _PTHREAD_T_DECLARED
struct pthread_mutex;  /* Forward declaration only */
struct pthread;        /* Forward declaration only */
#endif

/* Don't redefine __mbstate_t if it's already defined by FreeBSD headers */
#if !defined(___MBSTATE_T_DECLARED) && !defined(_MBSTATE_T_DECLARED_) && !defined(_SYS__TYPES_H_)
typedef struct {
    int __count;
    union {
        int __wch;
        char __wchb[4];
    } __value;
} __mbstate_t;
#define ___MBSTATE_T_DECLARED
#endif

/* stdio buffer structure */
struct __sbuf {
    unsigned char *_base;
    int _size;
};

/* Complete FreeBSD FILE structure */
struct __sFILE {
    unsigned char *_p;      /* (*) current position in (some) buffer */
    int _r;                 /* (*) read space left for getc() */
    int _w;                 /* (*) write space left for putc() */
    short _flags;           /* (*) flags, below; this FILE is free if 0 */
    short _file;            /* (*) fileno, if Unix descriptor, else -1 */
    struct __sbuf _bf;      /* (*) the buffer (at least 1 byte, if !NULL) */
    int _lbfsize;           /* (*) 0 or -_bf._size, for inline putc */

    /* operations */
    void *_cookie;          /* (*) cookie passed to io functions */
    int (*_close)(void *);
    int (*_read)(void *, char *, int);
    fpos_t (*_seek)(void *, fpos_t, int);
    int (*_write)(void *, const char *, int);

    /* separate buffer for long sequences of ungetc() */
    struct __sbuf _ub;      /* ungetc buffer */
    unsigned char *_up;     /* saved _p when _p is doing ungetc data */
    int _ur;                /* saved _r when _r is counting ungetc data */

    /* tricks to meet minimum requirements even when malloc() fails */
    unsigned char _ubuf[3]; /* guarantee an ungetc() buffer */
    unsigned char _nbuf[1]; /* guarantee a getc() buffer */

    /* separate buffer for fgetln() when line crosses buffer boundary */
    struct __sbuf _lb;      /* buffer for fgetln() */

    /* Unix stdio files get aligned to block boundaries on fseek() */
    int _blksize;           /* stat.st_blksize (may be != _bf._size) */
    fpos_t _offset;         /* current lseek offset */

    pthread_mutex_t *_fl_mutex; /* used for MT-safety */
    pthread_t *_fl_owner;        /* current owner */
    int _fl_count;               /* recursive lock count */
    int _orientation;            /* orientation for fwide() */
    __mbstate_t _mbstate;        /* multibyte conversion state */
    int _flags2;                 /* additional flags */
};

/* FILE type definition */
typedef struct __sFILE FILE;

/* FreeBSD FILE flags - needed for proper operation */
#define __SLBF  0x0001      /* line buffered */
#define __SNBF  0x0002      /* unbuffered */
#define __SRD   0x0004      /* OK to read */
#define __SWR   0x0008      /* OK to write */
#define __SRW   0x0010      /* open for reading & writing */
#define __SEOF  0x0020      /* found EOF */
#define __SERR  0x0040      /* found error */
#define __SMBF  0x0080      /* _bf._base is from malloc */
#define __SAPP  0x0100      /* fdopen()ed in append mode */
#define __SSTR  0x0200      /* this is an sprintf/snprintf string */
#define __SOPT  0x0400      /* do fseek() optimization */
#define __SNPT  0x0800      /* do not do fseek() optimization */
#define __SOFF  0x1000      /* set iff _offset is in fact correct */
#define __SMOD  0x2000      /* true => fgetln modified _p text */
#define __SALC  0x4000      /* allocate string space dynamically */
#define __SIGN  0x8000      /* ignore this file in _fwalk */

#define __S2OAP 0x0001      /* O_APPEND mode is set */

/* Standard buffer sizes */
#ifndef BUFSIZ
#define BUFSIZ 1024         /* Smaller for embedded systems */
#endif

/* Standard file descriptor constants */
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

/* EOF definition */
#ifndef EOF
#define EOF (-1)
#endif

/* Seek constants */
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

/*******************************************************************************
 * SECTION: Mount System Call Compatibility
 *
 * Redirects FreeBSD's `mount` system call to a Unikraft-compatible wrapper.
 * This is necessary because the function signatures differ between the two
 * systems.
 ******************************************************************************/

/* Only apply mount compatibility when compiling Unikraft VFS code that includes sys/mount.h */
#ifdef _SYS_MOUNT_H_   /* FreeBSD mount.h guard */

/* Undefine FreeBSD mount to avoid conflicts */
#ifdef mount
#undef mount
#endif

/* Provide Unikraft-compatible mount function signature */
int mount(const char *source, const char *target, const char *filesystemtype,
          unsigned long mountflags, const void *data);

/* Compatibility wrapper for FreeBSD-style mount calls (if needed) */
static inline int freebsd_mount(const char *type, const char *dir, int flags, void *data) {
    /* Convert FreeBSD mount call to Unikraft mount call */
    return mount("", dir, type ? type : "ramfs", (unsigned long)flags, data);
}

#endif /* _SYS_MOUNT_H_ */

/*******************************************************************************
 * SECTION: Final Compatibility Layer
 *
 * This section performs final checks to ensure that the compatibility layer
 * has been applied correctly and that critical constants are defined as
 * expected.
 ******************************************************************************/

/*
 * CRITICAL: We have replaced FreeBSD's sys/_clock_id.h with our own version
 * that ensures CLOCK_BOOTTIME has a unique value and does not alias CLOCK_MONOTONIC.
 * This prevents "duplicate case value" compilation errors in Unikraft's time.c
 */

/* Verify our clock constants are correctly defined */
#if defined(CLOCK_BOOTTIME) && defined(CLOCK_MONOTONIC)
#if CLOCK_BOOTTIME == CLOCK_MONOTONIC
#error "CLOCK_BOOTTIME must not be defined as an alias to CLOCK_MONOTONIC - this causes duplicate case values"
#endif
#endif

/* Ensure CLOCK_MONOTONIC_RAW is always available */
#ifndef CLOCK_MONOTONIC_RAW
#define CLOCK_MONOTONIC_RAW 14
#endif

/* Mark that this compatibility layer was successfully applied */
#define _FREEBSD_UNIKRAFT_COMPAT_APPLIED 1

#endif /* _FREEBSD_COMPAT_H_ */