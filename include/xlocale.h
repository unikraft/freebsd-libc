/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal xlocale shim. The port has one baked-in C locale, so locale_t is an
 * opaque pointer, the _l variants are forward-declared (defined by the FreeBSD
 * stdlib sources), and the locale-aware ctype predicates map to their locale-free
 * <ctype.h> counterparts. Nothing here is FreeBSD-derived.
 */
#ifndef _XLOCALE_H_
#define _XLOCALE_H_

/* stddef.h FIRST: <stdlib.h> below chains into nolibc -> uk/alloc.h ->
 * uk/assert.h -> uk/print.h, which references NULL. A consumer that includes
 * <locale.h>/<xlocale.h> before <stddef.h> (e.g. vfprintf.c) would otherwise
 * hit "NULL undeclared" deep in that Unikraft header chain. */
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/* nolibc (via <sys/types.h> -> shareddefs.h) may already typedef locale_t as
 * `struct __locale_struct *` when __NEED_locale_t is set. Respect its guard so
 * we don't redefine it with a different underlying type - either definition is
 * fine for us (locale_t is opaque here; every _l function ignores it). */
#if !defined(__DEFINED_locale_t) && !defined(_LOCALE_T_DECLARED)
typedef void *locale_t;
#define __DEFINED_locale_t
#define _LOCALE_T_DECLARED
#endif

long           strtol_l(const char *, char **, int, locale_t);
unsigned long  strtoul_l(const char *, char **, int, locale_t);
long long      strtoll_l(const char *, char **, int, locale_t);
unsigned long long strtoull_l(const char *, char **, int, locale_t);
intmax_t       strtoimax_l(const char *, char **, int, locale_t);
uintmax_t      strtoumax_l(const char *, char **, int, locale_t);
int            atoi_l(const char *, locale_t);
long           atol_l(const char *, locale_t);
long long      atoll_l(const char *, locale_t);
/* gdtoa machdep_ldisx.c defines strtold_l; stdlib/strtold.c calls it. The
 * long-double return type is load-bearing: without this decl strtold.c assumes
 * an int return and reads the wrong register, corrupting the result. */
long double    strtold_l(const char *, char **, locale_t);

#define isspace_l(c, loc)   ((void)(loc), isspace(c))
#define isalpha_l(c, loc)   ((void)(loc), isalpha(c))
#define isdigit_l(c, loc)   ((void)(loc), isdigit(c))
#define isupper_l(c, loc)   ((void)(loc), isupper(c))
#define islower_l(c, loc)   ((void)(loc), islower(c))
#define isalnum_l(c, loc)   ((void)(loc), isalnum(c))
#define ispunct_l(c, loc)   ((void)(loc), ispunct(c))
#define isxdigit_l(c, loc)  ((void)(loc), isxdigit(c))
#define toupper_l(c, loc)   ((void)(loc), toupper(c))
#define tolower_l(c, loc)   ((void)(loc), tolower(c))

#ifdef __cplusplus
}
#endif

#endif /* _XLOCALE_H_ */
