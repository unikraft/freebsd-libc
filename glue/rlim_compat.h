/*
 * Resource limit type compatibility layer
 * 
 * This header resolves conflicts between FreeBSD libc and musl rlim_t definitions
 * when both need to coexist in the same build (due to Unikraft library dependencies)
 */

#ifndef _RLIM_COMPAT_H_
#define _RLIM_COMPAT_H_

/* If FreeBSD libc is active but musl headers are also needed */
#ifdef CONFIG_LIBFREEBSD

/* Save any existing rlim_t definition */
#ifdef _RLIM_T_DECLARED
#define FREEBSD_RLIM_T_WAS_DECLARED 1
#endif

/* Temporarily undefine to allow musl definition */
#ifdef rlim_t
#undef rlim_t
#endif

#ifdef _RLIM_T_DECLARED
#undef _RLIM_T_DECLARED
#endif

/* Allow musl to define its version for compatibility */
typedef unsigned long long musl_rlim_t;

/* Map rlim_t to musl version for mixed environments */
#ifndef FREEBSD_RLIM_T_WAS_DECLARED
typedef musl_rlim_t rlim_t;
#define _RLIM_T_DECLARED 1
#endif

#endif /* CONFIG_LIBFREEBSD */

#endif /* _RLIM_COMPAT_H_ */