/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <collate.h> shim. C locale only (no collation tables): the struct
 * carries just __collate_load_error, which our __get_locale() sets to 1 so
 * vfscanf/fnmatch take the plain byte-compare path; the range/symbol helpers
 * are linker-only stubs in glue/stubs.c. Pulls the locale shim in at the bottom
 * (fnmatch derefs __get_locale() through this header).
 */
#ifndef _COLLATE_FREEBSDPORT_SHIM_H_
#define _COLLATE_FREEBSDPORT_SHIM_H_

#include <wchar.h>		/* wchar_t, mbstate_t, size_t */

#define	COLLATE_STR_LEN	24	/* matches FreeBSD lib/libc/locale/collate.h */

struct xlocale_collate {
	int __collate_load_error;
};

extern int __collate_range_cmp(int, int);

/*
 * Wide range compare + bracket-subexpression helpers. Only referenced by
 * fnmatch (and never on the runtime path, since __collate_load_error == 1).
 * Prototypes mirror FreeBSD's collate.h exactly; C-locale stubs live in
 * glue/stubs.c.
 */
int	__wcollate_range_cmp(wchar_t, wchar_t);
size_t	__collate_collating_symbol(wchar_t *, size_t, const char *, size_t,
	    mbstate_t *);
int	__collate_equiv_class(const char *, size_t, mbstate_t *);
size_t	__collate_equiv_match(int, wchar_t *, size_t, wchar_t, const char *,
	    size_t, mbstate_t *, size_t *);

/*
 * regex/regcomp.c (findmust) references _CurrentRuneLocale->__encoding once,
 * guarded by `MB_CUR_MAX > 1` - always false in this C-locale-only port
 * (MB_CUR_MAX == 1), so the dereference is dead code. It must still compile and
 * link: expose a minimal rune locale whose encoding is the C-locale value
 * "NONE". The object is defined in glue/stubs.c. (We use our own struct tag -
 * regex only ever touches the ->__encoding member, never the type name.)
 */
struct __fbsdport_runelocale { char __encoding[33]; };
extern struct __fbsdport_runelocale *_CurrentRuneLocale;

/*
 * Pull in the fake-C-locale handle last, after struct xlocale_collate is
 * defined: <xlocale_private.h> includes us back, but its guard is already set
 * by then, so the struct is visible when it declares __fbsdport_C_collate.
 */
#include <xlocale_private.h>

#endif /* _COLLATE_FREEBSDPORT_SHIM_H_ */
