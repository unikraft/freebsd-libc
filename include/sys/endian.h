/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <sys/endian.h> shim. nolibc ships none, but gdtoa's fpmath.h needs
 * the byte-order macros for its IEEE bit-field unions. Targets (x86_64/arm64)
 * are little-endian, so that's baked in; the byte-swap helpers are omitted.
 */
#ifndef _SYS_ENDIAN_FREEBSDPORT_SHIM_H_
#define _SYS_ENDIAN_FREEBSDPORT_SHIM_H_

#define _LITTLE_ENDIAN 1234
#define _BIG_ENDIAN    4321
#define _PDP_ENDIAN    3412

#ifndef _BYTE_ORDER
#define _BYTE_ORDER _LITTLE_ENDIAN
#endif

/* Non-underscore aliases (some sources use the unprefixed names). */
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN _LITTLE_ENDIAN
#define BIG_ENDIAN    _BIG_ENDIAN
#define PDP_ENDIAN    _PDP_ENDIAN
#define BYTE_ORDER    _BYTE_ORDER
#endif

#endif /* _SYS_ENDIAN_FREEBSDPORT_SHIM_H_ */
