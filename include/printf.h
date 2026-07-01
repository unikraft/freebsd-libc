/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Near-empty <printf.h> shim. vfprintf.c includes it but uses only the
 * __printflike attribute (from <sys/cdefs.h>). The registerable-printf
 * extension API is out of scope, so this declares none of it.
 */
#ifndef _PRINTF_FREEBSDPORT_SHIM_H_
#define _PRINTF_FREEBSDPORT_SHIM_H_

#include <sys/cdefs.h>

/* vfprintf.c gates the registerable-printf (xprintf) path on `__use_xprintf`.
 * We do not build xprintf, so it is defined = 0 in glue/stubs.c and the path
 * is dead. Declared here because vfprintf.c includes <printf.h>. */
extern int __use_xprintf;

#endif /* _PRINTF_FREEBSDPORT_SHIM_H_ */
