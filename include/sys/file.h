/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for <sys/file.h>. FreeBSD's flags.c needs the open(2) mode flags;
 * nolibc's <sys/file.h> has the flock() LOCK_* constants posix-fdio needs.
 * This sits on the global path, so it SUPPLEMENTS nolibc via #include_next
 * first, then adds the open flags + the FreeBSD-only O_VERIFY (a no-op here).
 */
#ifndef _SYS_FILE_FREEBSDPORT_OVERLAY_H_
#define _SYS_FILE_FREEBSDPORT_OVERLAY_H_

#include_next <sys/file.h>	/* nolibc: LOCK_SH/LOCK_EX/LOCK_NB/LOCK_UN, flock() */
#include <fcntl.h>		/* O_RDONLY/O_WRONLY/O_RDWR/O_CREAT/O_TRUNC/... */

#ifndef O_VERIFY
#define O_VERIFY 0
#endif

#endif /* _SYS_FILE_FREEBSDPORT_OVERLAY_H_ */
