/*
 * FreeBSD errno.h compatibility wrapper for Unikraft
 */
#ifndef _ERRNO_H_
#define _ERRNO_H_

#include <sys/errno.h>

/* Make errno variable available */
extern int errno;

#endif /* _ERRNO_H_ */
