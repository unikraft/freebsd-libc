/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <wchar.h> shim for the stdio port: just the types (wchar_t/wint_t/
 * mbstate_t) and the few prototypes the byte-oriented path needs. Wide stdio is
 * out of scope. mbstate_t is guarded by _MBSTATE_T_DECLARED and defined
 * identically in the stdio overlay (a layout mismatch would corrupt __sFILE).
 */
#ifndef _WCHAR_FREEBSDPORT_SHIM_H_
#define _WCHAR_FREEBSDPORT_SHIM_H_

#include <sys/cdefs.h>
#include <stddef.h>	/* size_t, NULL */

__BEGIN_DECLS

/* Under -nostdinc the <stddef.h> on the path doesn't hand us wchar_t in C
 * mode, so define it from the compiler builtin. (wchar_t is a keyword in
 * C++, so only typedef it for C.) */
#ifndef __cplusplus
#ifndef _WCHAR_T_DECLARED
typedef __WCHAR_TYPE__ wchar_t;
#define _WCHAR_T_DECLARED
#define __DEFINED_wchar_t	/* nolibc/musl guard, in case they also try */
#endif
#endif

#ifndef _WINT_T_DECLARED
typedef int wint_t;
#define _WINT_T_DECLARED
#endif

#ifndef _MBSTATE_T_DECLARED
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;	/* for alignment */
} __mbstate_t;
typedef __mbstate_t mbstate_t;
#define _MBSTATE_T_DECLARED
#endif

#ifndef WEOF
#define WEOF ((wint_t)-1)
#endif
#ifndef WCHAR_MIN
#define WCHAR_MIN (-2147483647 - 1)
#define WCHAR_MAX (2147483647)
#endif

/* The few byte-path helpers vfprintf's %ls conversion may reference. They are
 * declared (not defined) here; if a built source actually calls one, the
 * triage loop will pull in the corresponding FreeBSD source or a glue stub. */
size_t	wcslen(const wchar_t *);
wchar_t	*wmemchr(const wchar_t *, wchar_t, size_t);
size_t	mbrtowc(wchar_t * __restrict, const char * __restrict, size_t,
	    mbstate_t * __restrict);
size_t	wcrtomb(char * __restrict, wchar_t, mbstate_t * __restrict);

__END_DECLS

#endif /* _WCHAR_FREEBSDPORT_SHIM_H_ */
