/*
 * Minimal <arpa/nameser.h> overlay. The inet_*top/pton sources reference only
 * three size constants from it (no ns_* parser functions), so we provide just
 * those.
 */
#ifndef _LIBFREEBSD_ARPA_NAMESER_H_
#define _LIBFREEBSD_ARPA_NAMESER_H_

#define	NS_INT16SZ	2	/* bytes of data in a uint16_t */
#define	NS_INADDRSZ	4	/* IPv4 T_A */
#define	NS_IN6ADDRSZ	16	/* IPv6 T_AAAA */

#endif /* _LIBFREEBSD_ARPA_NAMESER_H_ */
