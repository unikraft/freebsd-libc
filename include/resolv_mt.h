/*
 * Minimal <resolv_mt.h> overlay. The only consumer (inet/nsap_addr.c) uses one
 * scratch buffer, inet_nsap_ntoa_tmpbuf; single-threaded, so a plain global
 * (defined in glue/stubs.c) is correct. Shadows the fork's resolv_mt.h.
 */
#ifndef _LIBFREEBSD_RESOLV_MT_H_
#define _LIBFREEBSD_RESOLV_MT_H_

extern char inet_nsap_ntoa_tmpbuf[255 * 3];

#endif /* _LIBFREEBSD_RESOLV_MT_H_ */
