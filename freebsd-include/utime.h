/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeBSD utime.h compatibility header for Unikraft
 * 
 * utime() functionality for file timestamp modification
 */

#ifndef _UTIME_H_
#define _UTIME_H_

#include <sys/types.h>
#include <time.h>

/* Structure for utime() */
struct utimbuf {
    time_t actime;   /* Access time */
    time_t modtime;  /* Modification time */
};

/* Function declarations */
int utime(const char *path, const struct utimbuf *times);

#endif /* _UTIME_H_ */