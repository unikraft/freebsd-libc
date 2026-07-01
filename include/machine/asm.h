/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <machine/asm.h> overlay. Provides only the directives amd64
 * _setjmp.S uses (ENTRY/END/WEAK_REFERENCE, + ALTENTRY/CNAME). This port builds
 * neither profiled nor PIC libc objects, so the plain non-PROF forms match.
 */
#ifndef _MACHINE_ASM_H_
#define _MACHINE_ASM_H_

#define CNAME(csym)		csym

#define _START_ENTRY		.text; .p2align 4,0x90

#define _ENTRY(x)		_START_ENTRY; \
				.globl CNAME(x); .type CNAME(x),@function; \
				CNAME(x):; .cfi_startproc

#define ALTENTRY(x)		_ENTRY(x)
#define ENTRY(x)		_ENTRY(x)

#define END(x)			.size x, . - x; .cfi_endproc

/* Create a weak reference alias from sym (both are C names). */
#define WEAK_REFERENCE(sym, alias)					\
	.weak CNAME(alias);						\
	.equ CNAME(alias),CNAME(sym)

#endif /* _MACHINE_ASM_H_ */
