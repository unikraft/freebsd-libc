/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * setjmp / longjmp / sigsetjmp / siglongjmp for the FreeBSD-libc-on-Unikraft
 * port, aliased onto the fast _setjmp / _longjmp from the FreeBSD amd64
 * lib/libc/amd64/gen/_setjmp.S.
 *
 * FreeBSD's real setjmp/sigsetjmp save the POSIX signal mask (they call
 * __libc_sigprocmask, and their setjmp.S also pulls in a "SYS.h" that the
 * extracted fork does not ship). A single-process Unikraft unikernel has no
 * per-thread signal mask to save or restore, so the mask-saving variants are
 * behaviourally identical to the fast ones here. We therefore implement them
 * as tail jumps into _setjmp / _longjmp.
 *
 * A tail `jmp` (no push/pop) leaves the stack untouched, so _setjmp still sees
 * the *caller's* return address at 0(%rsp) and saves the caller's %rsp - i.e.
 * the saved context is the caller of setjmp(), exactly as required. The second
 * argument to sigsetjmp (savemask) and siglongjmp is passed through / ignored.
 */
#include <machine/asm.h>

ENTRY(setjmp)
	jmp	_setjmp
END(setjmp)

ENTRY(sigsetjmp)			/* savemask arg in %rsi is ignored */
	jmp	_setjmp
END(sigsetjmp)

ENTRY(longjmp)
	jmp	_longjmp
END(longjmp)

ENTRY(siglongjmp)
	jmp	_longjmp
END(siglongjmp)

	.section .note.GNU-stack,"",%progbits
