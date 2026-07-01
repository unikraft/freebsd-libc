/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for <sys/stat.h>. FreeBSD's fopen.c uses DEFFILEMODE, which nolibc
 * lacks. Global-path header: SUPPLEMENT nolibc via #include_next first, then add
 * only the missing BSD convenience macros.
 */
#ifndef _SYS_STAT_FREEBSDPORT_OVERLAY_H_
#define _SYS_STAT_FREEBSDPORT_OVERLAY_H_

#include_next <sys/stat.h>

/* All access bits (rwx for u/g/o). */
#ifndef ACCESSPERMS
#define ACCESSPERMS (S_IRWXU | S_IRWXG | S_IRWXO)	/* 0777 */
#endif
/* Default file creation mode: rw for all. */
#ifndef DEFFILEMODE
#define DEFFILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) /* 0666 */
#endif
#ifndef ALLPERMS
#define ALLPERMS (S_ISUID | S_ISGID | S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO) /* 07777 */
#endif

#endif /* _SYS_STAT_FREEBSDPORT_OVERLAY_H_ */
