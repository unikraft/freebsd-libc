/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Thread-local storage backing for the single-threaded pthread shim
 * (see include/pthread.h). The unikernel runs one cooperative thread, so
 * "thread-local" collapses to a single global slot table. OpenSSL's error
 * stack and init machinery create a key in one translation unit and read it
 * back in another, so this table must be ONE definition shared image-wide -
 * hence a .c file rather than static inlines in the header.
 *
 * pthread_once / mutex / rwlock are caller-state-only and live as inline
 * no-ops in the header; only key/get/set/delete need shared storage.
 */

#include <pthread.h>
#include <errno.h>

/*
 * With Unikraft pthread-embedded (PTE) enabled, the real pthread_key_* are
 * provided by PTE - gate ours out to avoid a multiple-definition link error
 * (same discipline as the env/posix-user gates in stubs.c).
 */
#if !defined(CONFIG_LIBPTHREAD_EMBEDDED)

#ifndef PTHREAD_SHIM_MAX_KEYS
#define PTHREAD_SHIM_MAX_KEYS 64
#endif

static struct {
	void *value;
	void (*destructor)(void *);
	int   used;
} tls_slots[PTHREAD_SHIM_MAX_KEYS];

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
	for (unsigned int i = 0; i < PTHREAD_SHIM_MAX_KEYS; i++) {
		if (!tls_slots[i].used) {
			tls_slots[i].used = 1;
			tls_slots[i].value = (void *)0;
			tls_slots[i].destructor = destructor;
			*key = i;
			return 0;
		}
	}
	return EAGAIN;
}

int pthread_key_delete(pthread_key_t key)
{
	if (key >= PTHREAD_SHIM_MAX_KEYS || !tls_slots[key].used)
		return EINVAL;
	/*
	 * POSIX does not run destructors on pthread_key_delete; OpenSSL frees
	 * its per-thread state explicitly before deleting the key. Just drop
	 * the slot.
	 */
	tls_slots[key].used = 0;
	tls_slots[key].value = (void *)0;
	tls_slots[key].destructor = (void (*)(void *))0;
	return 0;
}

void *pthread_getspecific(pthread_key_t key)
{
	if (key >= PTHREAD_SHIM_MAX_KEYS || !tls_slots[key].used)
		return (void *)0;
	return tls_slots[key].value;
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
	if (key >= PTHREAD_SHIM_MAX_KEYS || !tls_slots[key].used)
		return EINVAL;
	tls_slots[key].value = (void *)value;
	return 0;
}

#endif /* !CONFIG_LIBPTHREAD_EMBEDDED */
