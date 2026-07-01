/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <ssp/ssp.h> shim. With _FORTIFY_SOURCE off, the only macro that
 * matters is __ssp_real(fn) (fread.c uses it), which must expand to the bare
 * symbol name. The bounds-checking macros are no-ops.
 */
#ifndef _SSP_SSP_FREEBSDPORT_SHIM_H_
#define _SSP_SSP_FREEBSDPORT_SHIM_H_

#ifndef __ssp_real
#define __ssp_real(fn)		fn
#define __ssp_real_(fn)		fn
#endif

#define __ssp_bos(ptr)		((__SIZE_TYPE__)-1)
#define __ssp_bos0(ptr)		((__SIZE_TYPE__)-1)
#define __ssp_check(buf, len, bos)	((void)0)
#define __ssp_redirect_raw(rtype, fn, args, call, cond, bos)	/* nothing */
#define __ssp_redirect(rtype, fn, args, call)			/* nothing */
#define __ssp_redirect0(rtype, fn, args, call)			/* nothing */
#define __ssp_inline	static __inline __attribute__((__always_inline__))

#endif /* _SSP_SSP_FREEBSDPORT_SHIM_H_ */
