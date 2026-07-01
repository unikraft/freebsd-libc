/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal overlay for FreeBSD 15's <exterr.h>. gen/err.c references only
 * uexterr_gettext(), so we declare that alone and skip the error-category
 * cascade (glue stub reports "none"; err()/warn() fall back to strerror()).
 */
#ifndef _EXTERR_H_
#define _EXTERR_H_

#include <sys/cdefs.h>
#include <stddef.h>		/* size_t */

__BEGIN_DECLS
int uexterr_gettext(char *buf, size_t bufsz);
__END_DECLS

#endif /* _EXTERR_H_ */
