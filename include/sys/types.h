/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for nolibc's sys/types.h. FreeBSD sources expect <sys/types.h> to
 * make the stdint.h family visible (the BSD convention); nolibc's doesn't, so
 * we chain via #include_next and then pull in <stdint.h> (our overlay).
 */
#ifndef _SYS_TYPES_FREEBSDPORT_OVERLAY_H_
#define _SYS_TYPES_FREEBSDPORT_OVERLAY_H_

#include_next <sys/types.h>
#include <stdint.h>
/* FreeBSD's <sys/types.h> transitively exposes the <sys/limits.h> extension
 * macros (OFF_MAX/OFF_MIN/LONG_BIT). stdio.c relies on OFF_MAX and reaches it
 * only through here (via local.h's `#include <sys/types.h>`), so pull it in. */
#include <sys/limits.h>

#ifndef _U_INT_TYPES_DEFINED_
#define _U_INT_TYPES_DEFINED_
typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
typedef unsigned long long u_quad_t;
typedef long long          quad_t;
typedef quad_t *           qaddr_t;
typedef int32_t            register_t;
typedef int32_t            __register_t;
/* Fixed-width BSD aliases. FreeBSD's <sys/types.h> defines these; nolibc does
 * not. The inet sources (inet_addr/ntop/pton) use the u_int8/16/32_t forms. */
typedef uint8_t            u_int8_t;
typedef uint16_t           u_int16_t;
typedef uint32_t           u_int32_t;
typedef uint64_t           u_int64_t;
#endif

#endif /* _SYS_TYPES_FREEBSDPORT_OVERLAY_H_ */
