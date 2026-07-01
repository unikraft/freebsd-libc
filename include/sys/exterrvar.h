/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal overlay for FreeBSD 15's <sys/exterrvar.h>. gen/err.c needs only
 * UEXTERROR_MAXLEN from it, so we provide just that and skip the kernel
 * extended-error cascade (uexterr_gettext() is a glue stub; see glue/stubs.c).
 */
#ifndef _SYS_EXTERRVAR_H_
#define _SYS_EXTERRVAR_H_

#define UEXTERROR_MAXLEN 256	/* matches FreeBSD sys/sys/exterrvar.h */

#endif /* _SYS_EXTERRVAR_H_ */
