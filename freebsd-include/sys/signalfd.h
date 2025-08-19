/*
 * FreeBSD sys/signalfd.h stub for Unikraft
 */
#ifndef _SYS_SIGNALFD_H_
#define _SYS_SIGNALFD_H_
#include <signal.h>

#define SFD_CLOEXEC 02000000
#define SFD_NONBLOCK 04000

struct signalfd_siginfo {
    uint32_t ssi_signo;
    int32_t ssi_errno;
    int32_t ssi_code;
    uint32_t ssi_pid;
    uint32_t ssi_uid;
    int32_t ssi_fd;
    uint32_t ssi_tid;
    uint32_t ssi_band;
    uint32_t ssi_overrun;
    uint32_t ssi_trapno;
    int32_t ssi_status;
    int32_t ssi_int;
    uint64_t ssi_ptr;
    uint64_t ssi_utime;
    uint64_t ssi_stime;
    uint64_t ssi_addr;
    uint8_t pad[128 - 12*4 - 4*8];
};

int signalfd(int fd, const sigset_t *mask, int flags);

#endif /* _SYS_SIGNALFD_H_ */
