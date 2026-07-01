/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for nolibc's inttypes.h. nolibc declares intmax_t / uintmax_t
 * but not imaxdiv_t, which FreeBSD's lib/libc/stdlib/imaxdiv.c requires.
 * This shim pulls in nolibc's header and adds what is missing.
 */
#ifndef _INTTYPES_FREEBSDPORT_OVERLAY_H_
#define _INTTYPES_FREEBSDPORT_OVERLAY_H_

#include_next <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _IMAXDIV_T_DEFINED_
#define _IMAXDIV_T_DEFINED_
typedef struct {
	intmax_t	quot;
	intmax_t	rem;
} imaxdiv_t;
#endif

#ifdef __cplusplus
}
#endif

#endif /* _INTTYPES_FREEBSDPORT_OVERLAY_H_ */
