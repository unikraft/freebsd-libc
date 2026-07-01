/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay <libgen.h> (nolibc has none). Declares the two POSIX path helpers we
 * build (gen/basename.c, dirname.c). FreeBSD 15 ships the POSIX.1-2008 in-place
 * variants, so the prototypes take a writable char *.
 */
#ifndef _LIBGEN_FREEBSDPORT_OVERLAY_H_
#define _LIBGEN_FREEBSDPORT_OVERLAY_H_

/*
 * basename.c/dirname.c return their result via __DECONST (from the cdefs shim),
 * which casts through uintptr_t - but they include only <libgen.h> + <string.h>.
 * Pull <stdint.h> in here so uintptr_t is visible in exactly those TUs.
 */
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

char *basename(char *);
char *dirname(char *);

#ifdef __cplusplus
}
#endif

#endif /* _LIBGEN_FREEBSDPORT_OVERLAY_H_ */
