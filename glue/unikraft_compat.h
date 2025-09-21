/*
 * Unikraft/FreeBSD Compatibility Header
 *
 * This header is force-included by the build system. It is the very
 * first header to be included by any compilation unit.
 *
 * Its purpose is to resolve definition conflicts between the Unikraft
 * code base and the FreeBSD libc headers.
 */

#ifndef __UNIKRAFT_FREEBSD_COMPAT_H__
#define __UNIKRAFT_FREEBSD_COMPAT_H__

/*
 * Unikraft's <uk/compiler.h> defines a set of macros that are also
 * defined by FreeBSD's <sys/cdefs.h>. We solve this by undefining
 * the Unikraft version before any FreeBSD header has a chance to
 * see them. The FreeBSD versions are compatible, so this is safe.
 */
#ifdef __unused
#undef __unused
#endif

#ifdef __used
#undef __used
#endif

#ifdef __packed
#undef __packed
#endif

#ifdef __section
#undef __section
#endif

#ifdef __noinline
#undef __noinline
#endif

#ifdef __offsetof
#undef __offsetof
#endif

#ifdef __containerof
#undef __containerof
#endif

/*
 * Unikraft's <uk/arch/limits.h> and FreeBSD's <machine/_limits.h>
 * both define __OFF_MAX and __OFF_MIN but with different values,
 * leading to a conflict. We undefine the Unikraft version.
 */
#ifdef __OFF_MAX
#undef __OFF_MAX
#endif

#ifdef __OFF_MIN
#undef __OFF_MIN
#endif

/*
 * Same issue for CACHE_LINE_SIZE, MIN, and MAX.
 */
#ifdef CACHE_LINE_SIZE
#undef CACHE_LINE_SIZE
#endif

#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

#endif /* __UNIKRAFT_FREEBSD_COMPAT_H__ */
