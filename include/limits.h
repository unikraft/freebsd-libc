/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for nolibc's limits.h. nolibc does not define the BSD-extension
 * macros LONG_BIT / WORD_BIT that several FreeBSD source files (strspn,
 * strpbrk, ...) reference via plain <limits.h>.
 */
#ifndef _LIMITS_FREEBSDPORT_OVERLAY_H_
#define _LIMITS_FREEBSDPORT_OVERLAY_H_

#include_next <limits.h>

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

/* Max positional printf args (`%N$`). FreeBSD gets this from <sys/syslimits.h>
 * via <limits.h>; printf-pos.c uses it to size its argument-type table. */
#ifndef NL_ARGMAX
#define NL_ARGMAX 4096
#endif

/* Multibyte / message-catalog limits vfprintf.c references (also normally from
 * <sys/syslimits.h>). The values are the FreeBSD defaults. */
#ifndef MB_LEN_MAX
#define MB_LEN_MAX 6
#endif
#ifndef NL_TEXTMAX
#define NL_TEXTMAX 2048
#endif

/* POSIX2 regex limit: max count in a \{m,n\} bound. regex/utils.h derives
 * DUPMAX/INFINITY from it. FreeBSD gets it from <sys/syslimits.h> via <limits.h>;
 * the POSIX-mandated minimum (and FreeBSD's value) is 255. */
#ifndef _POSIX2_RE_DUP_MAX
#define _POSIX2_RE_DUP_MAX 255
#endif

#endif /* _LIMITS_FREEBSDPORT_OVERLAY_H_ */
