#ifndef _SIGINFO_H_
#define _SIGINFO_H_

/* FreeBSD siginfo_t compatibility header */
#include <sys/signal.h>

#ifndef _SIGINFO_T_DECLARED
typedef struct __siginfo siginfo_t;
#define _SIGINFO_T_DECLARED
#endif

#endif /* _SIGINFO_H_ */
