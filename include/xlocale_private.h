/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal xlocale_private.h shim. One C locale, no threads/tables. Provides
 * FIX_LOCALE (no-op) and __get_locale(), which returns a static C locale whose
 * collate component reports __collate_load_error = 1, so vfscanf stays on its
 * byte-compare path (see <collate.h>). The _l functions ignore the handle.
 */
#ifndef _XLOCALE_PRIVATE_H_
#define _XLOCALE_PRIVATE_H_

#include <xlocale.h>
#include <collate.h>

#define FIX_LOCALE(loc) ((void)(loc))

/* Component indices into _xlocale.components[]. Order matches FreeBSD; only
 * XLC_COLLATE is actually populated here. */
enum {
	XLC_COLLATE = 0,
	XLC_CTYPE,
	XLC_MONETARY,
	XLC_NUMERIC,
	XLC_TIME,
	XLC_MESSAGES,
	XLC_LAST
};

struct _xlocale {
	void *components[XLC_LAST];
};

static struct xlocale_collate __fbsdport_C_collate = { 1 }; /* load_error = 1 */
static struct _xlocale __fbsdport_C_locale = {
	{ &__fbsdport_C_collate, 0, 0, 0, 0, 0 }
};

static inline struct _xlocale *__get_locale(void)
{
	return &__fbsdport_C_locale;
}

#endif /* _XLOCALE_PRIVATE_H_ */
