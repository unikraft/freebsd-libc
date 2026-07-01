/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for nolibc's stdint.h. Adds intmax_t/uintmax_t and the
 * int_leastN_t/int_fastN_t families that FreeBSD sources, snprintf and SQLite
 * rely on. Chains through nolibc's header.
 */
#ifndef _STDINT_FREEBSDPORT_OVERLAY_H_
#define _STDINT_FREEBSDPORT_OVERLAY_H_

#include_next <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _INTMAX_T_DEFINED_
#define _INTMAX_T_DEFINED_
typedef long long          intmax_t;
typedef unsigned long long uintmax_t;
#endif

#ifndef _INT_LEAST_DEFINED_
#define _INT_LEAST_DEFINED_
typedef int8_t   int_least8_t;
typedef int16_t  int_least16_t;
typedef int32_t  int_least32_t;
typedef int64_t  int_least64_t;
typedef uint8_t  uint_least8_t;
typedef uint16_t uint_least16_t;
typedef uint32_t uint_least32_t;
typedef uint64_t uint_least64_t;

typedef int8_t   int_fast8_t;
typedef int16_t  int_fast16_t;
typedef int32_t  int_fast32_t;
typedef int64_t  int_fast64_t;
typedef uint8_t  uint_fast8_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
typedef uint64_t uint_fast64_t;
#endif

#ifndef PTRDIFF_MIN
#if __SIZEOF_POINTER__ == 8
#define PTRDIFF_MIN INT64_MIN
#define PTRDIFF_MAX INT64_MAX
#else
#define PTRDIFF_MIN INT32_MIN
#define PTRDIFF_MAX INT32_MAX
#endif
#endif

#ifndef WCHAR_MIN
#define WCHAR_MIN INT32_MIN
#define WCHAR_MAX INT32_MAX
#endif

#ifndef WINT_MIN
#define WINT_MIN  0U
#define WINT_MAX  UINT32_MAX
#endif

#ifndef SIG_ATOMIC_MIN
#define SIG_ATOMIC_MIN INT32_MIN
#define SIG_ATOMIC_MAX INT32_MAX
#endif

#ifdef __cplusplus
}
#endif

#endif /* _STDINT_FREEBSDPORT_OVERLAY_H_ */
