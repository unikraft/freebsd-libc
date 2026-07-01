/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <sys/param.h> shim. The stdio sources use only a few of its macros
 * (MIN, howmany, roundup, MAXBSIZE), so we provide those plus common siblings
 * and chain to <sys/types.h>, rather than adopting FreeBSD's kernel-heavy one.
 */
#ifndef _SYS_PARAM_FREEBSDPORT_SHIM_H_
#define _SYS_PARAM_FREEBSDPORT_SHIM_H_

/* GLOBAL-path header: SUPPLEMENT nolibc's <sys/param.h>, don't replace it -
 * chain through first so vfscore keeps MAXPATHLEN/MAXSYMLINKS. We then add
 * the few math macros the FreeBSD
 * stdio sources use that nolibc's param.h lacks. */
#include_next <sys/param.h>
#include <sys/types.h>
#include <limits.h>

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef howmany
#define howmany(x, y) (((x) + ((y) - 1)) / (y))
#endif
#ifndef rounddown
#define rounddown(x, y) (((x) / (y)) * (y))
#endif
#ifndef roundup
#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#endif
#ifndef powerof2
#define powerof2(x) ((((x) - 1) & (x)) == 0)
#endif

/* Largest file-system block size stdio will pick for a buffer. */
#ifndef MAXBSIZE
#define MAXBSIZE 65536
#endif

#endif /* _SYS_PARAM_FREEBSDPORT_SHIM_H_ */
