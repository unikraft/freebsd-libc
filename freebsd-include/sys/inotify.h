/*
 * FreeBSD sys/inotify.h stub for Unikraft
 */
#ifndef _SYS_INOTIFY_H_
#define _SYS_INOTIFY_H_
#include <stdint.h>

#define IN_CLOEXEC 02000000
#define IN_NONBLOCK 04000
#define IN_ACCESS 0x00000001
#define IN_MODIFY 0x00000002
#define IN_ATTRIB 0x00000004
#define IN_CREATE 0x00000100
#define IN_DELETE 0x00000200

struct inotify_event {
    int wd;
    uint32_t mask;
    uint32_t cookie;
    uint32_t len;
    char name[];
};

int inotify_init(void);
int inotify_init1(int flags);
int inotify_add_watch(int fd, const char *pathname, uint32_t mask);
int inotify_rm_watch(int fd, int wd);

#endif /* _SYS_INOTIFY_H_ */
