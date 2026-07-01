/*
 * Minimal <arpa/inet.h> overlay (musl-imported ships none). Provides the inet_*
 * prototypes and replicates FreeBSD's rename scheme (#define inet_X __inet_X),
 * which each source needs for its __weak_reference(__inet_X, inet_X). Types come
 * from the existing musl-imported <netinet/in.h>/<sys/socket.h>.
 */
#ifndef _LIBFREEBSD_ARPA_INET_H_
#define _LIBFREEBSD_ARPA_INET_H_

#include <stddef.h>		/* NULL (inet_addr.c uses it but includes no stdio) */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define	inet_addr		__inet_addr
#define	inet_aton		__inet_aton
#define	inet_lnaof		__inet_lnaof
#define	inet_makeaddr		__inet_makeaddr
#define	inet_neta		__inet_neta
#define	inet_netof		__inet_netof
#define	inet_network		__inet_network
#define	inet_net_ntop		__inet_net_ntop
#define	inet_net_pton		__inet_net_pton
#define	inet_cidr_ntop		__inet_cidr_ntop
#define	inet_cidr_pton		__inet_cidr_pton
#define	inet_ntoa		__inet_ntoa
#define	inet_ntoa_r		__inet_ntoa_r
#define	inet_pton		__inet_pton
#define	inet_ntop		__inet_ntop
#define	inet_nsap_addr		__inet_nsap_addr
#define	inet_nsap_ntoa		__inet_nsap_ntoa

__BEGIN_DECLS
in_addr_t	 inet_addr(const char *);
char		*inet_ntoa(struct in_addr);
const char	*inet_ntop(int, const void * __restrict, char * __restrict,
		    socklen_t);
int		 inet_pton(int, const char * __restrict, void * __restrict);

int		 inet_aton(const char *, struct in_addr *);
in_addr_t	 inet_lnaof(struct in_addr);
struct in_addr	 inet_makeaddr(in_addr_t, in_addr_t);
char		*inet_neta(in_addr_t, char *, size_t);
in_addr_t	 inet_netof(struct in_addr);
in_addr_t	 inet_network(const char *);
char		*inet_net_ntop(int, const void *, int, char *, size_t);
int		 inet_net_pton(int, const char *, void *, size_t);
char		*inet_ntoa_r(struct in_addr, char *, socklen_t);
char		*inet_cidr_ntop(int, const void *, int, char *, size_t);
int		 inet_cidr_pton(int, const char *, void *, int *);
unsigned	 inet_nsap_addr(const char *, unsigned char *, int);
char		*inet_nsap_ntoa(int, const unsigned char *, char *);
__END_DECLS

#endif /* _LIBFREEBSD_ARPA_INET_H_ */
