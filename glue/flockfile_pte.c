/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * stdio FILE-locking shim for the real-threads (PTE) configuration.
 *
 * Replaces FreeBSD's lib/libc/stdio/_flock_stub.c when Unikraft pthread-
 * embedded is enabled (gated in Makefile.uk). The upstream file identifies the
 * locking thread by comparing FILE->_fl_owner (a `struct pthread *`) against
 * _pthread_self(); under PTE pthread_t is `pte_handle_t` - a struct - which can
 * be neither compared with == nor stored in that pointer, so the upstream
 * source does not compile.
 *
 * This port runs FreeBSD stdio with __isthreaded == 0, so the FLOCKFILE /
 * FUNLOCKFILE macros never call these at runtime; they exist only to satisfy
 * the stdio sources at link time. They are therefore no-ops: stdio is
 * single-threaded-safe only (an application must not operate on the same FILE
 * from multiple PTE threads concurrently - the same documented limitation that
 * already applies to the gdtoa and stdio internal locks). The public
 * flockfile/funlockfile/ftrylockfile names are provided as weak aliases, just
 * as the upstream stub does.
 */
#include <uk/config.h>
#if defined(CONFIG_LIBPTHREAD_EMBEDDED)

#include <stdio.h>
#include <sys/cdefs.h>

void
_flockfile(FILE *fp)
{
	(void)fp;
}

void
_flockfile_debug_stub(FILE *fp, char *fname, int lineno)
{
	(void)fp;
	(void)fname;
	(void)lineno;
}

int
_ftrylockfile(FILE *fp)
{
	(void)fp;
	return 0;
}

void
_funlockfile(FILE *fp)
{
	(void)fp;
}

__weak_reference(_flockfile, flockfile);
__weak_reference(_flockfile_debug_stub, _flockfile_debug);
__weak_reference(_ftrylockfile, ftrylockfile);
__weak_reference(_funlockfile, funlockfile);

#endif /* CONFIG_LIBPTHREAD_EMBEDDED */
