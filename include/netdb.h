/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * <netdb.h> overlay. OpenSSL's socket BIO sources include it for struct
 * hostent/servent/addrinfo + getaddrinfo. nolibc's netdb.h is incomplete and
 * wins the include order, so we supply a self-contained POSIX version (layouts
 * match glibc/lwip). Name resolution isn't exercised - this is compile-only.
 */
#ifndef __NETDB_H__
#define __NETDB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/socket.h>   /* struct sockaddr, socklen_t */

/* nolibc declared this; keep it so existing consumers still link. */
extern int h_errno;

/* h_errno values for gethostbyname()/gethostbyaddr() */
#ifndef HOST_NOT_FOUND
#define HOST_NOT_FOUND 1
#define TRY_AGAIN      2
#define NO_RECOVERY    3
#define NO_DATA        4
#define NO_ADDRESS     NO_DATA
#endif

struct hostent {
	char  *h_name;        /* official name of host */
	char **h_aliases;     /* alias list */
	int    h_addrtype;    /* host address type */
	int    h_length;      /* length of address */
	char **h_addr_list;   /* list of addresses */
};
#define h_addr h_addr_list[0]

struct servent {
	char  *s_name;        /* official service name */
	char **s_aliases;     /* alias list */
	int    s_port;        /* port number (network byte order) */
	char  *s_proto;       /* protocol to use */
};

struct addrinfo {
	int               ai_flags;
	int               ai_family;
	int               ai_socktype;
	int               ai_protocol;
	socklen_t         ai_addrlen;
	struct sockaddr  *ai_addr;
	char             *ai_canonname;
	struct addrinfo  *ai_next;
};

/* getaddrinfo() ai_flags */
#ifndef AI_PASSIVE
#define AI_PASSIVE     0x01
#endif
#ifndef AI_CANONNAME
#define AI_CANONNAME   0x02
#endif
#ifndef AI_NUMERICHOST
#define AI_NUMERICHOST 0x04
#endif
#ifndef AI_NUMERICSERV
#define AI_NUMERICSERV 0x08
#endif
#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG  0x400
#endif

/* getnameinfo() limits and flags */
#ifndef NI_MAXHOST
#define NI_MAXHOST     1025
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV     32
#endif
#ifndef NI_NUMERICHOST
#define NI_NUMERICHOST 0x01
#endif
#ifndef NI_NUMERICSERV
#define NI_NUMERICSERV 0x02
#endif

/* getaddrinfo() error codes (subset referenced by OpenSSL) */
#ifndef EAI_SYSTEM
#define EAI_SYSTEM     -11
#endif

struct hostent *gethostbyname(const char *name);
struct servent *getservbyname(const char *name, const char *proto);
int getaddrinfo(const char *node, const char *service,
		const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo(struct addrinfo *res);
const char *gai_strerror(int errcode);
int getnameinfo(const struct sockaddr *sa, socklen_t salen,
		char *host, socklen_t hostlen,
		char *serv, socklen_t servlen, int flags);

#ifdef __cplusplus
}
#endif

#endif /* __NETDB_H__ */
