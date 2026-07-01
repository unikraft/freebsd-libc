/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal sys/cdefs.h for the FreeBSD libc port. FreeBSD sources use macros
 * from here - most importantly __weak_reference (creates weak aliases like
 * strchrnul -> __strchrnul). We don't adopt FreeBSD's full cdefs.h (it
 * conflicts with Unikraft's __unused/__packed/__aligned); just the pieces the
 * FreeBSD sources need.
 */
#ifndef _SYS_CDEFS_FREEBSDPORT_H_
#define _SYS_CDEFS_FREEBSDPORT_H_

#ifndef __BEGIN_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS   }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
#endif

#ifndef __THROW
#define __THROW
#endif

#ifndef __DECONST
#define __DECONST(type, var) ((type)(uintptr_t)(const void *)(var))
#endif

#ifndef __DEVOLATILE
#define __DEVOLATILE(type, var) ((type)(uintptr_t)(volatile void *)(var))
#endif

#ifndef __DEQUALIFY
#define __DEQUALIFY(type, var) ((type)(uintptr_t)(const volatile void *)(var))
#endif

/*
 * __weak_reference(sym, alias) makes `alias` a weak symbol equal to `sym`
 * (used across FreeBSD libc, e.g. strchrnul -> __strchrnul).
 */
#ifndef __weak_reference
#define __weak_reference(sym, alias)              \
	__asm__(".weak " #alias);                 \
	__asm__(".equ " #alias ", " #sym)
#endif

#ifndef __strong_reference
#define __strong_reference(sym, alias) \
	extern __typeof(sym) alias __attribute__((__alias__(#sym)))
#endif

#ifndef __weak_symbol
#define __weak_symbol __attribute__((__weak__))
#endif

#ifndef __dead2
#define __dead2 __attribute__((__noreturn__))
#endif

#ifndef __pure2
#define __pure2 __attribute__((__const__))
#endif

#ifndef __unreachable
#define __unreachable() __builtin_unreachable()
#endif

#ifndef __predict_true
#define __predict_true(exp)  __builtin_expect((exp), 1)
#define __predict_false(exp) __builtin_expect((exp), 0)
#endif

#ifndef __FBSDID
#define __FBSDID(s)
#endif

#ifndef __RCSID
#define __RCSID(s)
#endif

#ifndef __COPYRIGHT
#define __COPYRIGHT(s)
#endif

/* GCC has these as builtins; provide guards in case they are referenced. */
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#ifndef __has_extension
#define __has_extension __has_feature
#endif
#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __has_include
#define __has_include(x) 0
#endif

/*
 * FreeBSD visibility/feature gating macros. We're not gating anything;
 * just stub them so the headers/sources compile.
 */
#ifndef _Restrict_
#define _Restrict_ restrict
#endif

#ifndef __containerof
#define __containerof(x, s, m)                                    \
	__extension__({                                           \
		const __typeof(((s *)0)->m) *__mptr = (x);        \
		(s *)((const char *)__mptr - __builtin_offsetof(s, m)); \
	})
#endif

/*
 * Additions for the FreeBSD stdio port. stdio needs a wider slice of cdefs than
 * string/stdlib did: __printflike/__scanflike/__format_arg, the _Nullable
 * qualifiers + pragmas, __restrict, and the __*_VISIBLE gates. Defined
 * permissively (attributes real where useful, nullability expands away, all
 * gates on); harmless for non-stdio TUs, so left unconditional.
 */
/* FreeBSD emits a deprecation .gnu.warning note; expand it away (findfp.c
 * warns on the deprecated f_prealloc). */
#ifndef __warn_references
#define __warn_references(sym, msg)
#endif
#ifndef __sym_compat
#define __sym_compat(sym, impl, verid)
#endif
#ifndef __sym_default
#define __sym_default(sym, impl, verid)
#endif

#ifndef __printflike
#define __printflike(fmtarg, firstvararg) \
	__attribute__((__format__(__printf__, fmtarg, firstvararg)))
#endif
#ifndef __scanflike
#define __scanflike(fmtarg, firstvararg) \
	__attribute__((__format__(__scanf__, fmtarg, firstvararg)))
#endif
#ifndef __format_arg
#define __format_arg(fmtarg) __attribute__((__format_arg__(fmtarg)))
#endif
#ifndef __printf0like
#define __printf0like(fmtarg, firstvararg)
#endif

#ifndef __restrict
#if defined(__cplusplus) || !defined(__STDC_VERSION__) || \
    __STDC_VERSION__ < 199901
#define __restrict
#else
#define __restrict restrict
#endif
#endif

/* Nullability qualifiers (Clang feature). Expand away under GCC. */
#ifndef _Nullable
#define _Nullable
#endif
#ifndef _Nonnull
#define _Nonnull
#endif
#ifndef _Null_unspecified
#define _Null_unspecified
#endif
#ifndef __NULLABILITY_PRAGMA_PUSH
#define __NULLABILITY_PRAGMA_PUSH
#define __NULLABILITY_PRAGMA_POP
#endif

#ifndef __nonnull
/*
 * Unikraft's uk/compiler.h uses the bare `__nonnull` (no arg); FreeBSD's cdefs
 * uses the argument form `__nonnull(x)`. Both #ifndef-guard, first-seen wins.
 * This overlay is global, so Unikraft core sources (via lwip) would get the arg
 * form and fail on the bare usage. No FreeBSD source we build uses the arg form,
 * so we adopt Unikraft's bare form.
 *
 * Old line (kept for reference):
 * #define __nonnull(x)
 */
#define __nonnull __attribute__((__nonnull__))
#endif
#ifndef __malloc_like
#define __malloc_like __attribute__((__malloc__))
#endif
#ifndef __result_use_check
#define __result_use_check __attribute__((__warn_unused_result__))
#endif

/*
 * Feature-test visibility gates. We do not gate anything: declare the full
 * surface. (FreeBSD derives these from _POSIX_C_SOURCE/_XOPEN_SOURCE; this shim
 * just turns everything on.)
 */
#ifndef __POSIX_VISIBLE
#define __POSIX_VISIBLE 200809
#endif
#ifndef __XSI_VISIBLE
#define __XSI_VISIBLE 700
#endif
#ifndef __BSD_VISIBLE
#define __BSD_VISIBLE 1
#endif
#ifndef __ISO_C_VISIBLE
#define __ISO_C_VISIBLE 2011
#endif
#ifndef __EXT1_VISIBLE
#define __EXT1_VISIBLE 1
#endif

#endif /* _SYS_CDEFS_FREEBSDPORT_H_ */
