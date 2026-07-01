/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay <fmtmsg.h> for the FreeBSD-libc-on-Unikraft port (gen easy half).
 * nolibc ships no <fmtmsg.h>; FreeBSD gen/fmtmsg.c needs it. Standalone (just
 * the MM_* constants + the fmtmsg() prototype), mirrored verbatim from the fork.
 */
#ifndef _FMTMSG_H_
#define	_FMTMSG_H_

/* Source of condition is... */
#define	MM_HARD		0x0001	/* ...hardware. */
#define	MM_SOFT		0x0002	/* ...software. */
#define	MM_FIRM		0x0004	/* ...firmware. */

/* Condition detected by... */
#define	MM_APPL		0x0010	/* ...application. */
#define	MM_UTIL		0x0020	/* ...utility. */
#define	MM_OPSYS	0x0040	/* ...operating system. */

/* Display on... */
#define	MM_PRINT	0x0100	/* ...standard error. */
#define	MM_CONSOLE	0x0200	/* ...system console. */

#define	MM_RECOVER	0x1000	/* Recoverable error. */
#define	MM_NRECOV	0x2000	/* Non-recoverable error. */

/* Severity levels. */
#define	MM_NOSEV	0	/* No severity level provided. */
#define	MM_HALT		1	/* Error causing application to halt. */
#define	MM_ERROR	2	/* Non-fault fault. */
#define	MM_WARNING	3	/* Unusual non-error condition. */
#define	MM_INFO		4	/* Informative message. */

/* Null options. */
#define	MM_NULLLBL	(char *)0
#define	MM_NULLSEV	0
#define	MM_NULLMC	0L
#define	MM_NULLTXT	(char *)0
#define	MM_NULLACT	(char *)0
#define	MM_NULLTAG	(char *)0

/* Return values. */
#define	MM_OK		0	/* Success. */
#define	MM_NOMSG	1	/* Failed to output to stderr. */
#define	MM_NOCON	2	/* Failed to output to console. */
#define	MM_NOTOK	3	/* Failed to output anything. */

int	fmtmsg(long, const char *, int, const char *, const char *,
	    const char *);

#endif /* !_FMTMSG_H_ */
