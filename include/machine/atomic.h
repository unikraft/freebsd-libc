/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <machine/atomic.h> shim. The stdio sources use one atomic op,
 * atomic_set_rel_ptr (findfp.c). Single-threaded, so a plain store with a
 * compiler barrier is correct. Add more only if the build reports them missing.
 */
#ifndef _MACHINE_ATOMIC_FREEBSDPORT_SHIM_H_
#define _MACHINE_ATOMIC_FREEBSDPORT_SHIM_H_

#include <stdint.h>

#define __fbsdport_barrier() __asm__ __volatile__("" ::: "memory")

static inline void
atomic_set_rel_ptr(volatile uintptr_t *p, uintptr_t v)
{
	__fbsdport_barrier();
	*p = v;
}

static inline uintptr_t
atomic_load_acq_ptr(volatile uintptr_t *p)
{
	uintptr_t v = *p;
	__fbsdport_barrier();
	return v;
}

#endif /* _MACHINE_ATOMIC_FREEBSDPORT_SHIM_H_ */
