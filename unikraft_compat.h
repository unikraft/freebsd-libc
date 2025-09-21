/*
 * FreeBSD libc Unikraft Compatibility Header
 * 
 * This header provides compatibility definitions and redirections
 * for FreeBSD libc when building with Unikraft.
 * 
 * Include this header BEFORE any FreeBSD headers in glue layer code.
 */

#ifndef _UNIKRAFT_COMPAT_H_
#define _UNIKRAFT_COMPAT_H_

/* Unikraft environment markers */
#ifndef __UNIKRAFT__
#define __UNIKRAFT__ 1
#endif

#ifndef __UK__
#define __UK__ 1
#endif

/* FreeBSD compatibility definitions */
#ifndef __FreeBSD__
#define __FreeBSD__ 15  /* Version we're compatible with */
#endif

/* Prevent conflicts with Unikraft's own definitions */
#ifdef CONFIG_LIBUKLIBC
#error "FreeBSD libc cannot be used with uklibc - they are mutually exclusive"
#endif

#ifdef CONFIG_LIBMUSL  
#error "FreeBSD libc cannot be used with musl - they are mutually exclusive"
#endif

/* Architecture-specific definitions */
#if defined(__x86_64__) || defined(__amd64__)
#define __amd64__ 1
#elif defined(__aarch64__)
#define __aarch64__ 1
#elif defined(__arm__)
#define __arm__ 1
#endif

/* Compiler and toolchain compatibility */
#ifdef __GNUC__
#define __GNUCLIKE_ASM 3
#define __GNUCLIKE_ATTRIBUTE_MODE_DI 1
#endif

/* Memory and string function redirection placeholders */
/* These will be implemented in the glue layer */

#endif /* _UNIKRAFT_COMPAT_H_ */
