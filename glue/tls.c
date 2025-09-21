/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2023, Unikraft GmbH and The Unikraft Authors.
 * Licensed under the BSD-3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 */

#include <string.h>
#include <uk/thread.h>
#include <uk/tcb_impl.h>

/**
 * Initializes the thread-control block (TCB) pointer for a new thread.
 * This function is part of Unikraft's scheduler TCB initialization API.
 * For FreeBSD libc, we store the TCB pointer in the thread's private field.
 *
 * @param thread
 *   The Unikraft thread structure to initialize.
 * @param tcb
 *   A pointer to the thread-control block.
 * @return
 *   Always returns 0, indicating success.
 */
int uk_thread_uktcb_init(struct uk_thread *thread, void *tcb)
{
	/* TCB is stored in the private field */
	thread->priv = tcb;
	return 0;
}

/**
 * Finalizes the thread-control block (TCB) for a terminating thread.
 * This function is part of Unikraft's scheduler TCB finalization API.
 * Currently, no specific cleanup is required for FreeBSD libc.
 *
 * @param thread
 *   The Unikraft thread structure.
 * @param tcb
 *   A pointer to the thread-control block.
 */
void uk_thread_uktcb_fini(struct uk_thread *thread, void *tcb)
{
	/* Nothing to do */
	(void)thread;
	(void)tcb;
}

/*
 * Stack-Smashing Protector (SSP) Wrappers
 * When SSP is enabled (-fstack-protector-all), the compiler may replace calls
 * to `strlcpy` and `strlcat` with calls to their `__ssp_real_*` counterparts.
 * We provide these wrappers to ensure the symbols are available at link time.
 */
size_t __ssp_real_strlcpy(char *dst, const char *src, size_t size)
{
	return strlcpy(dst, src, size);
}

size_t __ssp_real_strlcat(char *dst, const char *src, size_t size)
{
	return strlcat(dst, src, size);
}

/**
 * Architecture-specific TCB initialization callback.
 * This function is called from low-level architecture code (e.g., context
 * switching) to initialize the TCB for a new thread. It is a critical link
 * between the Unikraft core and the C library's TLS implementation.
 *
 * @param tcb
 *   A pointer to the TCB to be initialized.
 */
void ukarch_tls_tcb_init(void *tcb)
{
	/*
	 * TODO: This is a stub implementation to satisfy the linker. A proper
	 * implementation is required for full thread-local storage support with
	 * FreeBSD libc, which would involve initializing FreeBSD-specific
	 * thread data structures here.
	 */
	(void)tcb;
}
