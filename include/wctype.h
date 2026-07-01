/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <wctype.h> shim. vfscanf's wide path includes it for wint_t and the
 * isw* predicates; C-locale only, so each maps to its ASCII <ctype.h>
 * counterpart on the low byte and is false for >= 128.
 */
#ifndef _WCTYPE_FREEBSDPORT_SHIM_H_
#define _WCTYPE_FREEBSDPORT_SHIM_H_

#include <ctype.h>

#ifndef _WINT_T_DECLARED
typedef int wint_t;
#define _WINT_T_DECLARED
#endif

#ifndef _WCTYPE_T_DECLARED
typedef unsigned long wctype_t;	/* matches FreeBSD */
#define _WCTYPE_T_DECLARED
#endif

#ifndef WEOF
#define WEOF ((wint_t)-1)
#endif

/* From FreeBSD <limits.h>; gen/fnmatch.c sizes a buffer with it for the
 * [[:class:]] bracket syntax. Guard-defined here so we needn't overlay limits. */
#ifndef CHARCLASS_NAME_MAX
#define CHARCLASS_NAME_MAX 14
#endif

/*
 * Named character classes. wctype("alpha") returns an opaque token; iswctype()
 * applies it. C-locale-only (the named classes map straight onto the ASCII
 * predicates above), defined in glue/stubs.c. Lets fnmatch's [[:alpha:]] et al.
 * work for real rather than being stubbed out.
 */
wctype_t wctype(const char *);
int	 iswctype(wint_t, wctype_t);

#define __fbsdport_isw(pred, wc) (((unsigned)(wc) < 128) && pred((int)(wc)))

#define iswalnum(wc)  __fbsdport_isw(isalnum,  wc)
#define iswalpha(wc)  __fbsdport_isw(isalpha,  wc)
#define iswblank(wc)  __fbsdport_isw(isblank,  wc)
#define iswcntrl(wc)  __fbsdport_isw(iscntrl,  wc)
#define iswdigit(wc)  __fbsdport_isw(isdigit,  wc)
#define iswgraph(wc)  __fbsdport_isw(isgraph,  wc)
#define iswlower(wc)  __fbsdport_isw(islower,  wc)
#define iswprint(wc)  __fbsdport_isw(isprint,  wc)
#define iswpunct(wc)  __fbsdport_isw(ispunct,  wc)
#define iswspace(wc)  __fbsdport_isw(isspace,  wc)
#define iswupper(wc)  __fbsdport_isw(isupper,  wc)
#define iswxdigit(wc) __fbsdport_isw(isxdigit, wc)

static __inline wint_t towlower(wint_t wc)
{ return ((unsigned)wc < 128) ? (wint_t)tolower((int)wc) : wc; }
static __inline wint_t towupper(wint_t wc)
{ return ((unsigned)wc < 128) ? (wint_t)toupper((int)wc) : wc; }

#endif /* _WCTYPE_FREEBSDPORT_SHIM_H_ */
