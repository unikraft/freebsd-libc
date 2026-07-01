/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for nolibc's <string.h>. Chains via #include_next and adds the few
 * decls FreeBSD sources need that nolibc omits - today strcoll() (used by
 * gen/glob.c; C-locale only, so strcoll == strcmp - see glue/stubs.c).
 */
#ifndef _STRING_FREEBSDPORT_OVERLAY_H_
#define _STRING_FREEBSDPORT_OVERLAY_H_

#include_next <string.h>

#ifdef __cplusplus
extern "C" {
#endif

int strcoll(const char *, const char *);

#ifdef __cplusplus
}
#endif

#endif /* _STRING_FREEBSDPORT_OVERLAY_H_ */
