/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for nolibc's assert.h. nolibc's assert(x) is a do/while statement,
 * which breaks in expression contexts like SQLite's `(assert(0), 0)`; ANSI C
 * requires a void expression, which this provides.
 */
#ifndef _ASSERT_FREEBSDPORT_OVERLAY_H_
#define _ASSERT_FREEBSDPORT_OVERLAY_H_

#include <uk/config.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef assert
#undef assert
#endif

extern void __assert_fail(const char *expr, const char *file,
			  unsigned int line, const char *func)
	__attribute__((__noreturn__));

#ifdef NDEBUG
#define assert(x) ((void)0)
#else
#define assert(x) \
	((void)((x) || (__assert_fail(#x, __FILE__, __LINE__, __func__), 0)))
#endif

#define static_assert _Static_assert

#ifdef __cplusplus
}
#endif

#endif /* _ASSERT_FREEBSDPORT_OVERLAY_H_ */
