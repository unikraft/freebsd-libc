/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeBSD compatibility shim for Linux prctl.h
 * This provides minimal compatibility for Unikraft
 */

#ifndef _SYS_PRCTL_H_
#define _SYS_PRCTL_H_

#include <sys/cdefs.h>

__BEGIN_DECLS

/*
 * Linux prctl operation constants - minimal set for compatibility
 * These are the most commonly used prctl operations
 */
#define PR_SET_PDEATHSIG  1  /* Set parent death signal */
#define PR_GET_PDEATHSIG  2  /* Get parent death signal */
#define PR_GET_DUMPABLE   3  /* Get dumpable flag */
#define PR_SET_DUMPABLE   4  /* Set dumpable flag */
#define PR_GET_UNALIGN    5  /* Get unaligned access control bits */
#define PR_SET_UNALIGN    6  /* Set unaligned access control bits */
#define PR_GET_KEEPCAPS   7  /* Get keep capabilities flag */
#define PR_SET_KEEPCAPS   8  /* Set keep capabilities flag */
#define PR_GET_FPEMU      9  /* Get floating point emulation control bits */
#define PR_SET_FPEMU      10 /* Set floating point emulation control bits */
#define PR_GET_FPEXC      11 /* Get floating point exception mode */
#define PR_SET_FPEXC      12 /* Set floating point exception mode */
#define PR_GET_TIMING     13 /* Get timing method */
#define PR_SET_TIMING     14 /* Set timing method */
#define PR_SET_NAME       15 /* Set process name */
#define PR_GET_NAME       16 /* Get process name */
#define PR_GET_ENDIAN     19 /* Get endianness */
#define PR_SET_ENDIAN     20 /* Set endianness */
#define PR_GET_SECCOMP    21 /* Get secure computing mode */
#define PR_SET_SECCOMP    22 /* Set secure computing mode */
#define PR_SET_TSC        26 /* Set TSC access */
#define PR_GET_TSC        27 /* Get TSC access */

/* prctl function declaration */
int prctl(int option, unsigned long arg2, unsigned long arg3,
          unsigned long arg4, unsigned long arg5);

__END_DECLS

#endif /* !_SYS_PRCTL_H_ */
