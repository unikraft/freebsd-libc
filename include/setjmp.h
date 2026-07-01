/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <setjmp.h> overlay. nolibc ships none, and PTE's <pthread.h> uses
 * setjmp/longjmp to start and unwind threads, so the port supplies it. jmp_buf
 * matches FreeBSD amd64 (long _jb[12]); the implementations are the amd64
 * _setjmp.S, with setjmp/sigsetjmp aliased onto _setjmp (no signal mask to save
 * on a single-process unikernel). amd64 only (_JBLEN is arch-specific).
 */
#ifndef _SETJMP_H_
#define _SETJMP_H_

#include <sys/cdefs.h>

#ifndef __returns_twice
#define __returns_twice __attribute__((__returns_twice__))
#endif

#define _JBLEN	12		/* Size of the jmp_buf on amd64. */

typedef	struct _jmp_buf    { long _jb[_JBLEN]; } jmp_buf[1];
typedef	struct _sigjmp_buf { long _sjb[_JBLEN]; } sigjmp_buf[1];

__BEGIN_DECLS
int	_setjmp(jmp_buf) __returns_twice;
void	_longjmp(jmp_buf, int) __dead2;
int	setjmp(jmp_buf) __returns_twice;
void	longjmp(jmp_buf, int) __dead2;
int	sigsetjmp(sigjmp_buf, int) __returns_twice;
void	siglongjmp(sigjmp_buf, int) __dead2;
__END_DECLS

#endif /* _SETJMP_H_ */
