/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Shim for FreeBSD sources that include <sys/limits.h>. nolibc has only
 * <limits.h> and lacks LONG_BIT/SSIZE_MAX, so this forwards to <limits.h> and
 * adds the BSD-extension macros the string routines use.
 */
#ifndef _SYS_LIMITS_H_
#define _SYS_LIMITS_H_

#include <limits.h>

#ifndef LONG_BIT
#if __LP64__ || __SIZEOF_LONG__ == 8
#define LONG_BIT 64
#else
#define LONG_BIT 32
#endif
#endif

#ifndef WORD_BIT
#define WORD_BIT 32
#endif

/* SSIZE_MAX is provided by nolibc's <stdint.h>. Don't redefine. */

#ifndef OFF_MAX
#define OFF_MAX LLONG_MAX
#endif

#ifndef OFF_MIN
#define OFF_MIN LLONG_MIN
#endif

#endif /* _SYS_LIMITS_H_ */
