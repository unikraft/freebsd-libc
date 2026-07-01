/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for nolibc's stdlib.h. nolibc does not declare div_t /
 * ldiv_t / lldiv_t, which FreeBSD's lib/libc/stdlib/{div,ldiv,lldiv}.c
 * require. This shim pulls in nolibc's header and adds what is missing.
 */
#ifndef _STDLIB_FREEBSDPORT_OVERLAY_H_
#define _STDLIB_FREEBSDPORT_OVERLAY_H_

#include_next <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _DIV_T_DEFINED_
#define _DIV_T_DEFINED_
typedef struct {
	int	quot;
	int	rem;
} div_t;

typedef struct {
	long	quot;
	long	rem;
} ldiv_t;

typedef struct {
	long long	quot;
	long long	rem;
} lldiv_t;
#endif

/*
 * FreeBSD's stdlib.h transitively makes ssize_t visible (it's used by
 * arc4random_buf, getline-style APIs, qsort internals, ...). nolibc keeps
 * ssize_t inside <sys/types.h> only, so qsort.c - which only includes
 * <stdlib.h> - would not see it. Expose it here.
 */
#ifndef _SSIZE_T_DECLARED
#define _SSIZE_T_DECLARED
typedef long ssize_t;
#endif

/*
 * String -> floating point. nolibc's stdlib.h declares none of these, so
 * without a prototype callers default strtod() to `int` and truncate the
 * result. strtod/strtof are provided by FreeBSD gdtoa; strtold/atof by glue.
 */
double      strtod(const char *, char **);
float       strtof(const char *, char **);
long double strtold(const char *, char **);
double      atof(const char *);

/*
 * Pulled in by gen/glob.c. nolibc's stdlib.h declares none of these:
 *   - MB_CUR_MAX: max bytes in a multibyte char. C locale only, so 1.
 *   - reallocarray: already built from FreeBSD stdlib/reallocarray.c (Phase A);
 *     this is just the missing prototype (else callers make a pointer from int).
 *   - secure_getenv: glob uses it for $HOME during ~ expansion; glue stub
 *     forwards to getenv() (which returns NULL - no environ in this port).
 */
#ifndef MB_CUR_MAX
#define MB_CUR_MAX ((size_t)1)
#endif
void *reallocarray(void *, size_t, size_t);
char *secure_getenv(const char *);

/* Provided by the real FreeBSD gen/{getprogname,setprogname}.c (gen easy half).
 * FreeBSD declares these in <stdlib.h>; nolibc's does not. */
const char *getprogname(void);
void setprogname(const char *);

/*
 * gen easy half: the rand48(3) PRNG family (real FreeBSD gen/*rand48*.c, pure
 * 48-bit LCG, no OS deps), plus fmtcheck(3) and getbsize(3). nolibc declares
 * none of these; without prototypes callers would default the double-returning
 * rand48 functions to int and truncate. drand48/erand48 use the real ldexp()
 * already defined in glue.
 */
double          drand48(void);
double          erand48(unsigned short[3]);
long            jrand48(unsigned short[3]);
void            lcong48(unsigned short[7]);
long            lrand48(void);
long            mrand48(void);
long            nrand48(unsigned short[3]);
unsigned short *seed48(unsigned short[3]);
void            srand48(long);

const char *fmtcheck(const char *, const char *);
char       *getbsize(int *, long *);

#ifdef __cplusplus
}
#endif

#endif /* _STDLIB_FREEBSDPORT_OVERLAY_H_ */
