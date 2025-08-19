#!/bin/bash

# FreeBSD Header Synchronization Script
# This script updates the curated headers from the FreeBSD source
# and applies any necessary compatibility patches

set -uo pipefail  # Remove -e to continue on missing headers

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
FREEBSD_LIBC_ROOT="$(dirname "$SCRIPT_DIR")"
FREEBSD_SOURCE_DIR="$FREEBSD_LIBC_ROOT/freebsd-src"
TARGET_INCLUDE_DIR="$FREEBSD_LIBC_ROOT/freebsd-include"
GLUE_DIR="$FREEBSD_LIBC_ROOT/glue"

echo "=== FreeBSD Header Synchronization ==="

# Check if FreeBSD source exists
if [ ! -d "$FREEBSD_SOURCE_DIR" ]; then
    echo "ERROR: FreeBSD source not found at $FREEBSD_SOURCE_DIR"
    echo "Please run install_freebsd_source.sh first"
    exit 1
fi

# Backup existing headers
if [ -d "$TARGET_INCLUDE_DIR" ]; then
    echo "Backing up existing headers..."
    mv "$TARGET_INCLUDE_DIR" "$TARGET_INCLUDE_DIR.backup.$(date +%Y%m%d_%H%M%S)"
fi

mkdir -p "$TARGET_INCLUDE_DIR"

# Define headers to sync - grouped by category for maintainability
declare -A HEADER_GROUPS=(
    [SYS_HEADERS]="sys/_types.h sys/cdefs.h sys/endian.h sys/param.h sys/types.h sys/limits.h sys/syslimits.h sys/stdint.h"
    [io]="sys/errno.h sys/stat.h sys/fcntl.h sys/unistd.h sys/uio.h unistd.h fcntl.h"
    [time]="sys/time.h time.h"
    [networking]="sys/socket.h sys/select.h netdb.h arpa/inet.h netinet/in.h"
    [stdlib]="stdlib.h stdio.h stddef.h stdint.h stdarg.h stdbool.h limits.h errno.h assert.h"
    [strings]="string.h strings.h memory.h"
    [locale]="ctype.h wctype.h locale.h xlocale.h runetype.h"
    [misc]="inttypes.h _ctype.h signal.h"
    [math]="math.h fenv.h"
    [threading]="pthread.h semaphore.h sched.h"
)

# Function to copy header with error handling
copy_header() {
    local header="$1"
    local src_header="$FREEBSD_SOURCE_DIR/include/$header"
    local dst_header="$TARGET_INCLUDE_DIR/$header"
    
    # Try include/ first, then sys/sys/ for system headers
    if [ ! -f "$src_header" ] && [[ "$header" == sys/* ]]; then
        # For sys/ headers, look in sys/sys/ directory
        local sys_header_name=$(basename "$header")
        src_header="$FREEBSD_SOURCE_DIR/sys/sys/$sys_header_name"
    fi
    
    if [ -f "$src_header" ]; then
        # Create directory structure if needed
        local dst_dir=$(dirname "$dst_header")
        mkdir -p "$dst_dir"
        
        # Copy the header
        cp "$src_header" "$dst_header"
        echo "  ✓ $header"
        return 0
    else
        echo "  ⚠ $header (not found, skipping)"
        return 1
    fi
}

# Sync headers by group
for group in "${!HEADER_GROUPS[@]}"; do
    echo "Syncing $group headers..."
    for header in ${HEADER_GROUPS[$group]}; do
        copy_header "$header"
    done
    echo ""
done

# Copy additional sys/ headers from kernel
echo "Syncing additional sys/ headers..."
SYS_HEADERS_SRC="$FREEBSD_SOURCE_DIR/sys/sys"
if [ -d "$SYS_HEADERS_SRC" ]; then
    mkdir -p "$TARGET_INCLUDE_DIR/sys"
    
    # Additional sys/ headers not in the main list
    # Include all the dependency headers for stdarg, stdint, and other essential headers  
    COMMON_SYS_HEADERS=(
        "queue.h" "tree.h" "_null.h" "_stdint.h" "_types.h" "_stdarg.h" 
        "_visible.h" "_pthreadtypes.h" "cdefs.h" "_endian.h" "bitcount.h"
        "_bitset.h" "bitset.h" "_callout.h" "_cpuset.h" "_lock.h" "_mutex.h"
        "_rwlock.h" "_sx.h" "_task.h" "_rmlock.h" "_condvar.h" "_eventhandler.h"
        "kassert.h" "systm.h" "kernel.h" "kobj.h" "linker_set.h" "_bus_dma.h"
        "bus.h" "callout.h" "cpu.h" "cpuset.h" "domain.h" "endian.h"
        "eventhandler.h" "file.h" "filedesc.h" "jail.h" "kdb.h" "ktr.h"
        "lock.h" "lockstat.h" "malloc.h" "mbuf.h" "mutex.h" "pcpu.h"
        "proc.h" "protosw.h" "refcount.h" "resourcevar.h" "rmlock.h"
        "rwlock.h" "sbuf.h" "sdt.h" "selinfo.h" "smp.h" "socket.h"
        "socketvar.h" "sx.h" "sysctl.h" "sysent.h" "taskqueue.h"
        "timetc.h" "ucred.h" "vnode.h" "timespec.h" "_sigval.h" "signal.h" "signal.h" "_sigaltstack.h" "_uio.h" "_param.h" "_maxphys.h" "_clock_id.h" "sched.h" "poll.h" "resource.h" "sysmacros.h" "ioctl.h" "ioccom.h" "filio.h" "sockio.h" "termios.h" "ttycom.h" "memrange.h" "gpio.h" "wait.h" "statfs.h" "membarrier.h" "mount.h" "statvfs.h" "dirent.h" "_winsize.h" "_ucontext.h"
    )
    for header in "${COMMON_SYS_HEADERS[@]}"; do
        if [ -f "$SYS_HEADERS_SRC/$header" ]; then
            cp "$SYS_HEADERS_SRC/$header" "$TARGET_INCLUDE_DIR/sys/"
            echo "  ✓ sys/$header"
        fi
    done
    
    # Copy any additional essential sys/ headers that might be dependencies
    # These are commonly needed by basic libc headers
    ADDITIONAL_SYS_HEADERS=(
        "_stddef.h" "_limits.h" "_timespec.h" "_timeval.h" "_iovec.h"
        "_sigset.h" "_sockaddr.h" "_fd_set.h" "_size_t.h" "_ssize_t.h"
        "_off_t.h" "_pid_t.h" "_uid_t.h" "_gid_t.h" "_mode_t.h"
        "_termios.h"
    )
    
    for header in "${ADDITIONAL_SYS_HEADERS[@]}"; do
        if [ -f "$SYS_HEADERS_SRC/$header" ]; then
            cp "$SYS_HEADERS_SRC/$header" "$TARGET_INCLUDE_DIR/sys/"
            echo "  ✓ sys/$header"
        fi
    done
fi

# Copy essential headers from other locations
echo "Syncing headers from other locations..."

# Copy compiler-essential headers that might be in different locations
# These are critical for platform compilation

# Try to find stdint.h in multiple locations
for search_dir in "$FREEBSD_SOURCE_DIR/include" "$FREEBSD_SOURCE_DIR/sys/sys" "$FREEBSD_SOURCE_DIR/contrib/gcc/ginclude"; do
    if [ -f "$search_dir/stdint.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/stdint.h" ]; then
        cp "$search_dir/stdint.h" "$TARGET_INCLUDE_DIR/"
        echo "  ✓ stdint.h (from $(basename $search_dir))"
    fi
done

# For stdarg.h, try architecture-specific locations first (critical for platform compilation)
STDARG_LOCATIONS=(
    "$FREEBSD_SOURCE_DIR/sys/amd64/include/stdarg.h"
    "$FREEBSD_SOURCE_DIR/sys/x86/include/stdarg.h"
    "$FREEBSD_SOURCE_DIR/sys/sys/stdarg.h"
    "$FREEBSD_SOURCE_DIR/include/stdarg.h"
    "$FREEBSD_SOURCE_DIR/contrib/llvm-project/clang/lib/Headers/stdarg.h"
)

for stdarg_path in "${STDARG_LOCATIONS[@]}"; do
    if [ -f "$stdarg_path" ] && [ ! -f "$TARGET_INCLUDE_DIR/stdarg.h" ]; then
        cp "$stdarg_path" "$TARGET_INCLUDE_DIR/"
        echo "  ✓ stdarg.h (from $(basename $(dirname $stdarg_path)))"
        break
    fi
done

# Also check for stdint.h in architecture-specific locations
if [ ! -f "$TARGET_INCLUDE_DIR/stdint.h" ]; then
    STDINT_LOCATIONS=(
        "$FREEBSD_SOURCE_DIR/sys/amd64/include/stdint.h"
        "$FREEBSD_SOURCE_DIR/sys/x86/include/stdint.h"
        "$FREEBSD_SOURCE_DIR/sys/sys/stdint.h"
    )
    
    for stdint_path in "${STDINT_LOCATIONS[@]}"; do
        if [ -f "$stdint_path" ]; then
            cp "$stdint_path" "$TARGET_INCLUDE_DIR/"
            echo "  ✓ stdint.h (from $(basename $(dirname $stdint_path)))"
            break
        fi
    done
fi

# Copy other essential compiler headers
for search_dir in "$FREEBSD_SOURCE_DIR/include" "$FREEBSD_SOURCE_DIR/sys/sys"; do
    # Copy stdalign.h if available
    if [ -f "$search_dir/stdalign.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/stdalign.h" ]; then
        cp "$search_dir/stdalign.h" "$TARGET_INCLUDE_DIR/"
        echo "  ✓ stdalign.h (from $(basename $search_dir))"
    fi
    
    # Copy stdnoreturn.h if available
    if [ -f "$search_dir/stdnoreturn.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/stdnoreturn.h" ]; then
        cp "$search_dir/stdnoreturn.h" "$TARGET_INCLUDE_DIR/"
        echo "  ✓ stdnoreturn.h (from $(basename $search_dir))"
    fi
done

# Copy errno.h - might be in include/ or generated
if [ -f "$FREEBSD_SOURCE_DIR/include/errno.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/errno.h" ]; then
    cp "$FREEBSD_SOURCE_DIR/include/errno.h" "$TARGET_INCLUDE_DIR/"
    echo "  ✓ errno.h"
elif [ -f "$FREEBSD_SOURCE_DIR/sys/sys/errno.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/errno.h" ]; then
    # Create a simple errno.h that includes sys/errno.h
    cat > "$TARGET_INCLUDE_DIR/errno.h" << 'EOF'
/*
 * FreeBSD errno.h compatibility wrapper for Unikraft
 */
#ifndef _ERRNO_H_
#define _ERRNO_H_

#include <sys/errno.h>

/* Make errno variable available */
extern int errno;

#endif /* _ERRNO_H_ */
EOF
    echo "  ✓ errno.h (created wrapper)"
fi

# Copy fcntl.h - might be in include/ or needs a wrapper
if [ -f "$FREEBSD_SOURCE_DIR/include/fcntl.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/fcntl.h" ]; then
    cp "$FREEBSD_SOURCE_DIR/include/fcntl.h" "$TARGET_INCLUDE_DIR/"
    echo "  ✓ fcntl.h"
elif [ -f "$FREEBSD_SOURCE_DIR/sys/sys/fcntl.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/fcntl.h" ]; then
    # Create a simple fcntl.h that includes sys/fcntl.h
    cat > "$TARGET_INCLUDE_DIR/fcntl.h" << 'EOF'
/*
 * FreeBSD fcntl.h compatibility wrapper for Unikraft
 */
#ifndef _FCNTL_H_
#define _FCNTL_H_

#include <sys/fcntl.h>

#endif /* _FCNTL_H_ */
EOF
    echo "  ✓ fcntl.h (created wrapper)"
fi

# Create sys/epoll.h stub - epoll is Linux-specific, FreeBSD uses kqueue
if [ ! -f "$TARGET_INCLUDE_DIR/sys/epoll.h" ]; then
    mkdir -p "$TARGET_INCLUDE_DIR/sys"
    cat > "$TARGET_INCLUDE_DIR/sys/epoll.h" << 'EOF'
/*
 * FreeBSD sys/epoll.h stub for Unikraft
 * 
 * epoll is a Linux-specific interface. FreeBSD uses kqueue instead.
 * This provides complete epoll constants for compatibility with Unikraft.
 * 
 * Citations: Values from Unikraft lib/nolibc/include/sys/epoll.h
 */
#ifndef _SYS_EPOLL_H_
#define _SYS_EPOLL_H_

#include <stdint.h>

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#ifndef O_CLOEXEC
#define O_CLOEXEC 02000000
#endif

/* Flags for epoll_create1() */
#define EPOLL_CLOEXEC O_CLOEXEC

/* Valid opcodes to issue to epoll_ctl() */
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

/* Epoll event masks - Citation: Unikraft lib/nolibc/include/sys/epoll.h */
#define EPOLLIN     0x00000001  /* File descriptor is ready for read */
#define EPOLLPRI    0x00000002  /* Exceptional condition */
#define EPOLLOUT    0x00000004  /* File descriptor is ready for write */
#define EPOLLRDNORM 0x00000040  /* Equivalent to POLLIN */
#define EPOLLRDBAND 0x00000080  /* UNUSED */
#define EPOLLWRNORM 0x00000100  /* Equivalent to POLLOUT */
#define EPOLLWRBAND 0x00000200  /* UNUSED */
#define EPOLLMSG    0x00000400  /* UNUSED */
#define EPOLLRDHUP  0x00002000  /* Stream peer closed connection */

/* These events are set if they occur regardless of whether requested */
#define EPOLLERR    0x00000008  /* Error condition */
#define EPOLLHUP    0x00000010  /* Peer closed its end of channel */
#define EPOLLNVAL   0x00000020  /* Invalid request: fd not open */

/* Additional epoll flags */
#define EPOLLEXCLUSIVE  (1U << 28)  /* Set exclusive wakeup mode for fd */
#define EPOLLWAKEUP     (1U << 29)
#define EPOLLONESHOT    (1U << 30)  /* Set one-shot behavior for fd */
#define EPOLLET         (1U << 31)  /* Set edge-triggered behavior for fd */

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t events;        /* Epoll events */
    epoll_data_t data;      /* User data variable */
} __packed;

/* Function declarations - Citation: Unikraft lib/nolibc/include/sys/epoll.h */
int epoll_create(int size);
int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

#endif /* _SYS_EPOLL_H_ */
EOF
    echo "  ✓ sys/epoll.h (created complete stub with all constants)"
fi

# Create sys/prctl.h stub - prctl is Linux-specific  
if [ ! -f "$TARGET_INCLUDE_DIR/sys/prctl.h" ]; then
    mkdir -p "$TARGET_INCLUDE_DIR/sys"
    cat > "$TARGET_INCLUDE_DIR/sys/prctl.h" << 'EOF'
/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * FreeBSD compatibility shim for Linux prctl.h
 * This provides minimal compatibility for Unikraft
 */

#ifndef _SYS_PRCTL_H_
#define _SYS_PRCTL_H_

#include <sys/cdefs.h>

__BEGIN_DECLS

/*
 * Linux prctl operation constants - minimal set for compatibility
 * These are the most commonly used prctl operations
 */
#define PR_SET_PDEATHSIG  1  /* Set parent death signal */
#define PR_GET_PDEATHSIG  2  /* Get parent death signal */
#define PR_GET_DUMPABLE   3  /* Get dumpable flag */
#define PR_SET_DUMPABLE   4  /* Set dumpable flag */
#define PR_GET_UNALIGN    5  /* Get unaligned access control bits */
#define PR_SET_UNALIGN    6  /* Set unaligned access control bits */
#define PR_GET_KEEPCAPS   7  /* Get keep capabilities flag */
#define PR_SET_KEEPCAPS   8  /* Set keep capabilities flag */
#define PR_GET_FPEMU      9  /* Get floating point emulation control bits */
#define PR_SET_FPEMU      10 /* Set floating point emulation control bits */
#define PR_GET_FPEXC      11 /* Get floating point exception mode */
#define PR_SET_FPEXC      12 /* Set floating point exception mode */
#define PR_GET_TIMING     13 /* Get timing method */
#define PR_SET_TIMING     14 /* Set timing method */
#define PR_SET_NAME       15 /* Set process name */
#define PR_GET_NAME       16 /* Get process name */
#define PR_GET_ENDIAN     19 /* Get endianness */
#define PR_SET_ENDIAN     20 /* Set endianness */
#define PR_GET_SECCOMP    21 /* Get secure computing mode */
#define PR_SET_SECCOMP    22 /* Set secure computing mode */
#define PR_SET_TSC        26 /* Set TSC access */
#define PR_GET_TSC        27 /* Get TSC access */

/* prctl function declaration */
int prctl(int option, unsigned long arg2, unsigned long arg3,
          unsigned long arg4, unsigned long arg5);

__END_DECLS

#endif /* !_SYS_PRCTL_H_ */
EOF
    echo "  ✓ sys/prctl.h (created stub)"
fi

# Automated wrapper creation for missing headers
echo "Creating automated wrappers for missing headers..."

# Define header mappings: missing_header -> sys/actual_header (or custom content)
declare -A WRAPPER_HEADERS=(
    # Linux compatibility wrappers
    ["sys/epoll.h"]="EPOLL_STUB"
    ["sys/eventfd.h"]="EVENTFD_STUB" 
    ["sys/signalfd.h"]="SIGNALFD_STUB"
    ["sys/timerfd.h"]="TIMERFD_STUB"
    ["sys/inotify.h"]="INOTIFY_STUB"
    
    # Standard POSIX wrappers that FreeBSD handles differently
    ["poll.h"]="sys/poll.h"
    ["dirent.h"]="DIRENT_WRAPPER"
    ["termios.h"]="sys/termios.h"
    ["grp.h"]="GRP_WRAPPER"
    ["pwd.h"]="PWD_WRAPPER"
    
    # Network wrappers
    ["netinet/tcp.h"]="sys/netinet/tcp.h"
    ["netinet/udp.h"]="sys/netinet/udp.h" 
    ["net/if.h"]="sys/net/if.h"
)

create_wrapper() {
    local wrapper_path="$1"
    local content_type="$2"
    local wrapper_dir=$(dirname "$TARGET_INCLUDE_DIR/$wrapper_path")
    local wrapper_name=$(basename "$wrapper_path")
    local header_guard="_$(echo "$wrapper_path" | tr '/' '_' | tr '.' '_' | tr '[:lower:]' '[:upper:]')_"
    
    mkdir -p "$wrapper_dir"
    
    case "$content_type" in
        "EPOLL_STUB")
            cat > "$TARGET_INCLUDE_DIR/$wrapper_path" << 'EOF'
/*-
 * sys/epoll.h - Minimal epoll compatibility for FreeBSD
 */

#ifndef _SYS_EPOLL_H
#define _SYS_EPOLL_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

/* epoll event structure */
typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    uint32_t events;
    epoll_data_t data;
};

/* Event types - must match FreeBSD poll constants exactly */
#define EPOLLIN     0x0001    /* POLLIN - any readable data available */
#define EPOLLPRI    0x0002    /* POLLPRI - OOB/Urgent readable data */
#define EPOLLOUT    0x0004    /* POLLOUT - file descriptor is writeable */
#define EPOLLERR    0x0008    /* POLLERR - some poll error occurred */
#define EPOLLHUP    0x0010    /* POLLHUP - file descriptor was "hung up" */
#define EPOLLRDHUP  0x4000    /* POLLRDHUP - half shut down */

/* Additional epoll constants - must match poll constants */
#define EPOLLRDNORM    0x0040    /* POLLRDNORM - non-OOB/URG data available */
#define EPOLLRDBAND    0x0080    /* POLLRDBAND - OOB/Urgent readable data */
#define EPOLLWRNORM    0x0004    /* POLLWRNORM = POLLOUT - no write type differentiation */
#define EPOLLWRBAND    0x0100    /* POLLWRBAND - OOB/Urgent data can be written */
#define EPOLLMSG       0x400
#define EPOLLET        (1 << 31)    /* Edge Triggered */
#define EPOLLONESHOT   (1 << 30)    /* One-shot behavior */

/* epoll_create1() flags */
#define EPOLL_CLOEXEC   O_CLOEXEC

/* epoll_ctl() operations */
#define EPOLL_CTL_ADD   1
#define EPOLL_CTL_DEL   2
#define EPOLL_CTL_MOD   3

/* Function declarations */
int epoll_create(int size);
int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

__END_DECLS

#endif /* _SYS_EPOLL_H */

#define EPOLLIN     0x001
#define EPOLLOUT    0x004
#define EPOLLERR    0x008
#define EPOLLHUP    0x010
#define EPOLL_CTL_ADD 1
#define EPOLL_CTL_DEL 2
#define EPOLL_CTL_MOD 3

struct epoll_event {
    uint32_t events;
    union {
        void *ptr;
        int fd;
        uint32_t u32;
        uint64_t u64;
    } data;
};

int epoll_create(int size);
int epoll_create1(int flags);
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

#endif /* _SYS_EPOLL_H_ */
EOF
            ;;
        "EVENTFD_STUB")
            cat > "$TARGET_INCLUDE_DIR/$wrapper_path" << 'EOF'
/*
 * FreeBSD sys/eventfd.h stub for Unikraft
 */
#ifndef _SYS_EVENTFD_H_
#define _SYS_EVENTFD_H_
#include <stdint.h>

typedef uint64_t eventfd_t;
#define EFD_CLOEXEC 02000000
#define EFD_NONBLOCK 04000
#define EFD_SEMAPHORE 1

int eventfd(unsigned int initval, int flags);
int eventfd_read(int fd, eventfd_t *value);
int eventfd_write(int fd, eventfd_t value);

#endif /* _SYS_EVENTFD_H_ */
EOF
            ;;
        "SIGNALFD_STUB")
            cat > "$TARGET_INCLUDE_DIR/$wrapper_path" << 'EOF'
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
EOF
            ;;
        "TIMERFD_STUB")
            cat > "$TARGET_INCLUDE_DIR/$wrapper_path" << 'EOF'
/*
 * FreeBSD sys/timerfd.h stub for Unikraft
 */
#ifndef _SYS_TIMERFD_H_
#define _SYS_TIMERFD_H_
#include <time.h>

#define TFD_CLOEXEC 02000000
#define TFD_NONBLOCK 04000
#define TFD_TIMER_ABSTIME 1

int timerfd_create(int clockid, int flags);
int timerfd_settime(int fd, int flags, const struct itimerspec *new_value,
                   struct itimerspec *old_value);
int timerfd_gettime(int fd, struct itimerspec *curr_value);

#endif /* _SYS_TIMERFD_H_ */
EOF
            ;;
        "INOTIFY_STUB")
            cat > "$TARGET_INCLUDE_DIR/$wrapper_path" << 'EOF'
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
EOF
            ;;
        "DIRENT_WRAPPER")
            cat > "$TARGET_INCLUDE_DIR/$wrapper_path" << 'EOF'
/*
 * FreeBSD dirent.h wrapper for Unikraft
 */
#ifndef _DIRENT_H_
#define _DIRENT_H_
#include <sys/dirent.h>
#endif /* _DIRENT_H_ */
EOF
            ;;
        "GRP_WRAPPER")
            cat > "$TARGET_INCLUDE_DIR/$wrapper_path" << 'EOF'
/*
 * FreeBSD grp.h wrapper for Unikraft
 */
#ifndef _GRP_H_
#define _GRP_H_
#include <sys/types.h>

struct group {
    char *gr_name;
    char *gr_passwd;
    gid_t gr_gid;
    char **gr_mem;
};

struct group *getgrent(void);
struct group *getgrgid(gid_t gid);
struct group *getgrnam(const char *name);
int getgrgid_r(gid_t gid, struct group *grp, char *buffer, size_t bufsize, struct group **result);
int getgrnam_r(const char *name, struct group *grp, char *buffer, size_t bufsize, struct group **result);
void setgrent(void);
void endgrent(void);

#endif /* _GRP_H_ */
EOF
            ;;
        "PWD_WRAPPER")
            cat > "$TARGET_INCLUDE_DIR/$wrapper_path" << 'EOF'
/*
 * FreeBSD pwd.h wrapper for Unikraft
 */
#ifndef _PWD_H_
#define _PWD_H_
#include <sys/types.h>

struct passwd {
    char *pw_name;
    char *pw_passwd;
    uid_t pw_uid;
    gid_t pw_gid;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};

struct passwd *getpwent(void);
struct passwd *getpwuid(uid_t uid);
struct passwd *getpwnam(const char *name);
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);
int getpwnam_r(const char *name, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);
void setpwent(void);
void endpwent(void);

#endif /* _PWD_H_ */
EOF
            ;;
        *)
            # Generic wrapper that includes the mapped header
            cat > "$TARGET_INCLUDE_DIR/$wrapper_path" << EOF
/*
 * FreeBSD ${wrapper_name} wrapper for Unikraft
 */
#ifndef ${header_guard}
#define ${header_guard}
#include <${content_type}>
#endif /* ${header_guard} */
EOF
            ;;
    esac
    echo "  ✓ $wrapper_path (created wrapper)"
}

# Create all defined wrappers
for wrapper_header in "${!WRAPPER_HEADERS[@]}"; do
    if [ ! -f "$TARGET_INCLUDE_DIR/$wrapper_header" ]; then
        create_wrapper "$wrapper_header" "${WRAPPER_HEADERS[$wrapper_header]}"
    fi
done

# Automated missing header detection and creation
auto_create_missing_headers() {
    local test_build_dir="$1"
    if [ -z "$test_build_dir" ] || [ ! -d "$test_build_dir" ]; then
        echo "  ⚠ No test build directory provided for auto-detection"
        return
    fi
    
    echo "Running automated missing header detection..."
    local missing_headers=()
    local build_log=$(mktemp)
    
    # Run a quick build to detect missing headers
    if (cd "$test_build_dir" && timeout 60 make 2>&1 | grep -E "fatal error:|No such file or directory" | head -10) > "$build_log"; then
        while IFS= read -r line; do
            if [[ "$line" =~ fatal\ error:\ ([^:]+):\ No\ such\ file\ or\ directory ]]; then
                local missing_header="${BASH_REMATCH[1]}"
                missing_headers+=("$missing_header")
            fi
        done < "$build_log"
        
        # Auto-create missing headers
        for header in "${missing_headers[@]}"; do
            auto_create_header "$header"
        done
    fi
    
    rm -f "$build_log"
}

auto_create_header() {
    local header_name="$1"
    local header_path="$TARGET_INCLUDE_DIR/$header_name"
    local header_dir=$(dirname "$header_path")
    
    # Skip if header already exists
    [ -f "$header_path" ] && return
    
    mkdir -p "$header_dir"
    
    # Try to find the header in FreeBSD source
    local found_header=$(find "$FREEBSD_SOURCE_DIR" -name "$(basename "$header_name")" -type f | head -1)
    
    if [ -n "$found_header" ]; then
        # Copy found header
        cp "$found_header" "$header_path"
        echo "  ✓ $header_name (auto-copied from $(dirname "$found_header"))"
    else
        # Create a basic stub header
        local header_guard="_$(echo "$header_name" | tr '/' '_' | tr '.' '_' | tr '-' '_' | tr '[:lower:]' '[:upper:]')_"
        cat > "$header_path" << EOF
/*
 * Auto-generated stub for $header_name
 * This header was not found in FreeBSD source and needs manual implementation
 */
#ifndef $header_guard
#define $header_guard

#warning "Auto-generated stub header for $header_name - needs implementation"

#endif /* $header_guard */
EOF
        echo "  ⚠ $header_name (created stub - needs manual implementation)"
    fi
}

# Allow external trigger of auto-detection
if [ "${AUTO_DETECT_MISSING:-}" = "1" ] && [ -n "${BUILD_TEST_DIR:-}" ]; then
    auto_create_missing_headers "$BUILD_TEST_DIR"
fi

# Handle netinet/in.h - might be in different location
for search_dir in "$FREEBSD_SOURCE_DIR/sys/netinet" "$FREEBSD_SOURCE_DIR/include/netinet"; do
    if [ -f "$search_dir/in.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/netinet/in.h" ]; then
        mkdir -p "$TARGET_INCLUDE_DIR/netinet"
        cp "$search_dir/in.h" "$TARGET_INCLUDE_DIR/netinet/"
        echo "  ✓ netinet/in.h (from $(basename $(dirname $search_dir))/$(basename $search_dir))"
        break
    fi
done

# Handle math.h and fenv.h - might be in lib/msun or include
for search_dir in "$FREEBSD_SOURCE_DIR/lib/msun/src" "$FREEBSD_SOURCE_DIR/include"; do
    if [ -f "$search_dir/math.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/math.h" ]; then
        cp "$search_dir/math.h" "$TARGET_INCLUDE_DIR/"
        echo "  ✓ math.h (from $(basename $(dirname $search_dir))/$(basename $search_dir))"
    fi
    if [ -f "$search_dir/fenv.h" ] && [ ! -f "$TARGET_INCLUDE_DIR/fenv.h" ]; then
        cp "$search_dir/fenv.h" "$TARGET_INCLUDE_DIR/"
        echo "  ✓ fenv.h (from $(basename $(dirname $search_dir))/$(basename $search_dir))"
    fi
done

# Copy machine architecture headers
echo "Setting up machine architecture headers..."
MACHINE_SRC="$FREEBSD_SOURCE_DIR/sys/amd64/include"
MACHINE_BACKUP_DIR="$TARGET_INCLUDE_DIR/machine-backup"

if [ -d "$MACHINE_SRC" ]; then
    # Create machine-backup directory structure
    mkdir -p "$MACHINE_BACKUP_DIR/amd64"
    
    # Copy essential machine headers
    for header in endian.h param.h types.h _types.h limits.h _limits.h _stdint.h _align.h signal.h ucontext.h; do
        if [ -f "$MACHINE_SRC/$header" ]; then
            cp "$MACHINE_SRC/$header" "$MACHINE_BACKUP_DIR/amd64/"
            echo "  ✓ machine/$header (amd64)"
        fi
    done
    
    # Create the machine symlink
    if [ -L "$TARGET_INCLUDE_DIR/machine" ]; then
        rm "$TARGET_INCLUDE_DIR/machine"
    fi
    ln -sf machine-backup/amd64 "$TARGET_INCLUDE_DIR/machine"
    echo "  ✓ Created machine -> machine-backup/amd64 symlink"
    
    # Create machine/_inttypes.h symlink to x86 version if available
    if [ -f "$TARGET_INCLUDE_DIR/x86/_inttypes.h" ]; then
        if [ -L "$MACHINE_BACKUP_DIR/amd64/_inttypes.h" ] || [ -f "$MACHINE_BACKUP_DIR/amd64/_inttypes.h" ]; then
            rm "$MACHINE_BACKUP_DIR/amd64/_inttypes.h"
        fi
        # Use relative symlink to x86/_inttypes.h for machine/_inttypes.h
        ln -sf "../../x86/_inttypes.h" "$MACHINE_BACKUP_DIR/amd64/_inttypes.h"
        echo "  ✓ Created machine/_inttypes.h -> x86/_inttypes.h symlink"
    elif [ -f "$FREEBSD_SOURCE_DIR/sys/amd64/include/_inttypes.h" ]; then
        # Fallback: copy directly from amd64 source
        cp "$FREEBSD_SOURCE_DIR/sys/amd64/include/_inttypes.h" "$MACHINE_BACKUP_DIR/amd64/_inttypes.h"
        echo "  ✓ Copied machine/_inttypes.h from amd64 source"
    fi
fi

# Copy SSP (Stack Smashing Protection) headers
echo "Setting up SSP headers..."
SSP_SRC="$FREEBSD_SOURCE_DIR/include/ssp"
if [ -d "$SSP_SRC" ]; then
    mkdir -p "$TARGET_INCLUDE_DIR/ssp"
    
    for header in ssp.h strings.h string.h stdio.h stdlib.h unistd.h uio.h poll.h signal.h; do
        if [ -f "$SSP_SRC/$header" ]; then
            cp "$SSP_SRC/$header" "$TARGET_INCLUDE_DIR/ssp/"
            echo "  ✓ ssp/$header"
        fi
    done
fi

# Setup x86 architecture headers (needed by some wrapper headers)
echo "Setting up x86 architecture headers..."
X86_SRC="$FREEBSD_SOURCE_DIR/sys/x86/include"
if [ -d "$X86_SRC" ]; then
    mkdir -p "$TARGET_INCLUDE_DIR/x86"
    
    # Copy x86-specific headers that might be referenced, including the missing _align.h
    for header in stdarg.h endian.h types.h _types.h _limits.h _stdint.h signal.h _align.h _inttypes.h ucontext.h; do
        if [ -f "$X86_SRC/$header" ]; then
            # Create symlink instead of copy to maintain FreeBSD source integrity
            if [ -L "$TARGET_INCLUDE_DIR/x86/$header" ] || [ -f "$TARGET_INCLUDE_DIR/x86/$header" ]; then
                rm "$TARGET_INCLUDE_DIR/x86/$header"
            fi
            # Create relative symlink to FreeBSD source
            ln -sf "../../freebsd-src/sys/x86/include/$header" "$TARGET_INCLUDE_DIR/x86/$header"
            echo "  ✓ x86/$header (symlinked)"
        fi
    done
fi

# Setup xlocale headers using symlinks
echo "Setting up xlocale headers..."
XLOCALE_SRC="$FREEBSD_SOURCE_DIR/include/xlocale"
if [ -d "$XLOCALE_SRC" ]; then
    # Create xlocale directory if it doesn't exist
    mkdir -p "$TARGET_INCLUDE_DIR/xlocale"
    
    # Create symlinks to all xlocale headers
    for header in "$XLOCALE_SRC"/*.h; do
        if [ -f "$header" ]; then
            header_name=$(basename "$header")
            # Remove existing symlink or file
            if [ -L "$TARGET_INCLUDE_DIR/xlocale/$header_name" ] || [ -f "$TARGET_INCLUDE_DIR/xlocale/$header_name" ]; then
                rm "$TARGET_INCLUDE_DIR/xlocale/$header_name"
            fi
            # Create symlink to the original header
            ln -sf "../../freebsd-src/include/xlocale/$header_name" "$TARGET_INCLUDE_DIR/xlocale/$header_name"
            echo "  ✓ xlocale/$header_name (symlinked)"
        fi
    done
else
    echo "  ⚠ xlocale directory not found at $XLOCALE_SRC"
fi

# Create compatibility patches if needed
echo ""
echo "Applying compatibility patches..."

# Create a compatibility header that can be included to resolve conflicts
cat > "$TARGET_INCLUDE_DIR/unikraft_compat.h" << 'EOF'
/*
 * FreeBSD libc Unikraft Compatibility Header
 * 
 * This header provides compatibility definitions and redirections
 * for FreeBSD libc when building with Unikraft.
 * 
 * Include this header BEFORE any FreeBSD headers in glue layer code.
 */

#ifndef _UNIKRAFT_COMPAT_H_
#define _UNIKRAFT_COMPAT_H_

/* Unikraft environment markers */
#ifndef __UNIKRAFT__
#define __UNIKRAFT__ 1
#endif

#ifndef __UK__
#define __UK__ 1
#endif

/* FreeBSD compatibility definitions */
#ifndef __FreeBSD__
#define __FreeBSD__ 15  /* Version we're compatible with */
#endif

/* Prevent conflicts with Unikraft's own definitions */
#ifdef CONFIG_LIBUKLIBC
#error "FreeBSD libc cannot be used with uklibc - they are mutually exclusive"
#endif

#ifdef CONFIG_LIBMUSL  
#error "FreeBSD libc cannot be used with musl - they are mutually exclusive"
#endif

/* Architecture-specific definitions */
#if defined(__x86_64__) || defined(__amd64__)
#define __amd64__ 1
#elif defined(__aarch64__)
#define __aarch64__ 1
#elif defined(__arm__)
#define __arm__ 1
#endif

/* Compiler and toolchain compatibility */
#ifdef __GNUC__
#define __GNUCLIKE_ASM 3
#define __GNUCLIKE_ATTRIBUTE_MODE_DI 1
#endif

/* Memory and string function redirection placeholders */
/* These will be implemented in the glue layer */

#endif /* _UNIKRAFT_COMPAT_H_ */
EOF

echo "  ✓ Created unikraft_compat.h"

# Create a header dependency map for documentation
echo ""
echo "Creating header dependency documentation..."
cat > "$FREEBSD_LIBC_ROOT/docs/HEADERS.md" << EOF
# FreeBSD Headers Integration

## Header Categories

$(for group in "${!HEADER_GROUPS[@]}"; do
    echo "### $group"
    echo "\`\`\`"
    echo "${HEADER_GROUPS[$group]}" | tr ' ' '\n' | sed 's/^/- /'
    echo "\`\`\`"
    echo ""
done)

## Compatibility Strategy

1. **Direct Copy**: Most headers copied as-is from FreeBSD source
2. **Compatibility Layer**: \`unikraft_compat.h\` provides environment setup
3. **Glue Integration**: Glue layer handles implementation mapping
4. **Conflict Resolution**: Mutual exclusion with other libc implementations

## Update Process

Run \`scripts/sync_headers.sh\` to update headers from FreeBSD source.

## Last Updated

$(date -u '+%Y-%m-%d %H:%M:%S UTC')
EOF

# Create docs directory if it doesn't exist
mkdir -p "$FREEBSD_LIBC_ROOT/docs"

# Apply compatibility fixes for Unikraft integration
echo "Applying compatibility fixes..."

# Fix poll.h for complete Linux compatibility
if [[ -f "$TARGET_INCLUDE_DIR/sys/poll.h" ]]; then
    # Add SEEK constants if not present
    if ! grep -q "SEEK_SET" "$TARGET_INCLUDE_DIR/sys/poll.h"; then
        cat >> "$TARGET_INCLUDE_DIR/sys/poll.h" << 'EOF'

/* SEEK constants for Unikraft compatibility - ensure they're available in poll operations */
#ifndef SEEK_SET
#define SEEK_SET    0   /* set file offset to offset */
#define SEEK_CUR    1   /* set file offset to current plus offset */
#define SEEK_END    2   /* set file offset to EOF plus offset */
#endif

/* O_* constants for Unikraft compatibility - include fcntl.h */
#include <sys/fcntl.h>

EOF
        echo "  ✓ Added SEEK constants and fcntl.h include to sys/poll.h"
    fi
    
    # Fix poll constant values and add Linux compatibility aliases
    if ! grep -q "POLLINGINOF" "$TARGET_INCLUDE_DIR/sys/poll.h"; then
        # FreeBSD uses POLLIGNEOF, but some code expects POLLINGINOF
        # Add Linux compatibility alias after the POLLIGNEOF definition
        sed -i '/^#define.*POLLIGNEOF.*0x2000/a\
#define\tPOLLINGINOF\tPOLLIGNEOF\t/* Linux compatibility alias for POLLIGNEOF */' "$TARGET_INCLUDE_DIR/sys/poll.h"
        echo "  ✓ Added POLLINGINOF Linux compatibility alias to sys/poll.h"
    fi
    
    # Ensure POLLWRNORM has the correct Linux-compatible value
    if grep -q "POLLWRNORM.*POLLOUT" "$TARGET_INCLUDE_DIR/sys/poll.h"; then
        # Change POLLWRNORM from POLLOUT to 0x0100 for Linux compatibility
        sed -i 's/#define.*POLLWRNORM.*POLLOUT.*/#define\tPOLLWRNORM\t0x0100\t\t\/\* normal data may be written (Linux compat) \*\//' "$TARGET_INCLUDE_DIR/sys/poll.h"
        echo "  ✓ Fixed POLLWRNORM value for Linux compatibility in sys/poll.h"
    fi
    
    # Ensure POLLWRBAND has the correct value
    if ! grep -q "POLLWRBAND.*0x0200" "$TARGET_INCLUDE_DIR/sys/poll.h"; then
        # Update POLLWRBAND to match Linux value if it's different
        sed -i 's/#define.*POLLWRBAND.*0x0100.*/#define\tPOLLWRBAND\t0x0200\t\t\/\* priority data may be written (Linux compat) \*\//' "$TARGET_INCLUDE_DIR/sys/poll.h"
        echo "  ✓ Fixed POLLWRBAND value for Linux compatibility in sys/poll.h"
    fi
    
    # Update POLLRDHUP to match Unikraft compatibility (Unikraft uses 0x2000, FreeBSD uses 0x4000)
    if grep -q "POLLRDHUP.*0x4000" "$TARGET_INCLUDE_DIR/sys/poll.h"; then
        # Replace FreeBSD POLLRDHUP with Unikraft-compatible value - use more flexible pattern
        sed -i 's/POLLRDHUP[[:space:]]*0x4000/POLLRDHUP\t0x2000/' "$TARGET_INCLUDE_DIR/sys/poll.h"
        # Also update the comment
        sed -i 's/half shut down/half shut down (Unikraft compatible)/' "$TARGET_INCLUDE_DIR/sys/poll.h"
        echo "  ✓ Updated POLLRDHUP to Unikraft-compatible value (0x2000) in sys/poll.h"
    fi
fi




# Fix rlim_t compatibility in sys/types.h for mixed musl/FreeBSD environment
if [[ -f "$TARGET_INCLUDE_DIR/sys/types.h" ]]; then
    # Remove all existing rlim_t definitions to avoid conflicts
    if grep -q "typedef.*rlim_t" "$TARGET_INCLUDE_DIR/sys/types.h"; then
        # Remove all rlim_t typedef lines
        sed -i '/typedef.*rlim_t.*;.*/d' "$TARGET_INCLUDE_DIR/sys/types.h"
        # Add single consistent musl-compatible definition after the other typedefs
        sed -i '/typedef.*__rune_t.*rune_t/a\
typedef	unsigned long long	rlim_t;		/* resource limit - musl compatible */' "$TARGET_INCLUDE_DIR/sys/types.h"
        echo "  ✓ Unified rlim_t definition in sys/types.h (musl compatible)"
    fi
fi


# Fix rlim_t compatibility in sys/resource.h - simplified
if [[ -f "$TARGET_INCLUDE_DIR/sys/resource.h" ]]; then
    if grep -q "typedef.*__rlim_t.*rlim_t" "$TARGET_INCLUDE_DIR/sys/resource.h"; then
        # Use simple replacement to avoid sed issues
        sed -i 's/typedef.*__rlim_t.*rlim_t.*/typedef\tunsigned long long\trlim_t;\t\/\* musl compatible \*\//' "$TARGET_INCLUDE_DIR/sys/resource.h"
        echo "  ✓ Fixed rlim_t compatibility in sys/resource.h" 
    fi
fi

# Add missing Linux-compatibility O_NOATIME flag to sys/fcntl.h
if [[ -f "$TARGET_INCLUDE_DIR/sys/fcntl.h" ]]; then
    if ! grep -q "O_NOATIME" "$TARGET_INCLUDE_DIR/sys/fcntl.h"; then
        # Add O_NOATIME after O_DIRECT definition
        awk '/^#define.*O_DIRECT.*0x00010000/ {
            print $0
            print ""
            print "/* Linux compatibility flags */"
            print "#define\tO_NOATIME\t0x40000000\t/* Linux: do not update atime */"
            next
        } {print}' "$TARGET_INCLUDE_DIR/sys/fcntl.h" > "$TARGET_INCLUDE_DIR/sys/fcntl.h.tmp" && mv "$TARGET_INCLUDE_DIR/sys/fcntl.h.tmp" "$TARGET_INCLUDE_DIR/sys/fcntl.h"
        echo "  ✓ Added O_NOATIME compatibility flag to sys/fcntl.h"
    fi
fi

# Ensure sysmacros.h is always available
if [[ ! -f "$TARGET_INCLUDE_DIR/sys/sysmacros.h" ]]; then
    if [[ -f "$FREEBSD_SOURCE_DIR/sys/contrib/openzfs/lib/libspl/include/os/freebsd/sys/sysmacros.h" ]]; then
        cp "$FREEBSD_SOURCE_DIR/sys/contrib/openzfs/lib/libspl/include/os/freebsd/sys/sysmacros.h" "$TARGET_INCLUDE_DIR/sys/sysmacros.h"
        echo "  ✓ Added sys/sysmacros.h from OpenZFS"
    fi
fi

# Fix macro conflicts with Unikraft
echo "Fixing macro conflicts with Unikraft..."

# Fix __OFF_MAX and __OFF_MIN redefinition warnings in x86/_limits.h
if [[ -f "$TARGET_INCLUDE_DIR/x86/_limits.h" ]]; then
    if ! grep -q "Unikraft macro compatibility" "$TARGET_INCLUDE_DIR/x86/_limits.h"; then
        # Better approach: only define if not already defined by Unikraft
        sed -i '/\/\* Unikraft compatibility: only define if not already defined \*\//,/^#endif/c\
/* Unikraft compatibility: only define if not already defined by Unikraft or system */\
#if !defined(__OFF_MAX) || defined(__UK_REDEFINE_LIMITS__)\
#define	__OFF_MAX	__LONG_MAX	/* max value for an off_t */\
#endif\
#if !defined(__OFF_MIN) || defined(__UK_REDEFINE_LIMITS__)\
#define	__OFF_MIN	__LONG_MIN	/* min value for an off_t */\
#endif' "$TARGET_INCLUDE_DIR/x86/_limits.h"
        echo "  ✓ Added robust Unikraft __OFF_MAX/__OFF_MIN compatibility to x86/_limits.h"
    fi
fi

# Remove the broken duplicate fix - will handle this differently
# The sed command above was corrupting the file

# Fix MIN/MAX macro conflicts in sys/param.h
if [[ -f "$TARGET_INCLUDE_DIR/sys/param.h" ]]; then
    if ! grep -q "Unikraft MIN/MAX compatibility" "$TARGET_INCLUDE_DIR/sys/param.h"; then
        # Add conditional guards around MIN/MAX definitions
        sed -i '/^#define.*MIN(a,b)/i\
/* Unikraft MIN/MAX compatibility - prevent redefinition */\
#ifndef MIN' "$TARGET_INCLUDE_DIR/sys/param.h"
        
        sed -i '/^#define.*MIN(a,b)/a\
#endif' "$TARGET_INCLUDE_DIR/sys/param.h"
        
        sed -i '/^#define.*MAX(a,b)/i\
#ifndef MAX' "$TARGET_INCLUDE_DIR/sys/param.h"
        
        sed -i '/^#define.*MAX(a,b)/a\
#endif' "$TARGET_INCLUDE_DIR/sys/param.h"
        
        echo "  ✓ Added Unikraft MIN/MAX compatibility guards to sys/param.h"
    fi
fi

# Fix CACHE_LINE_SIZE conflict in machine/param.h
if [[ -f "$TARGET_INCLUDE_DIR/machine/param.h" ]]; then
    if ! grep -q "Unikraft CACHE_LINE_SIZE compatibility" "$TARGET_INCLUDE_DIR/machine/param.h"; then
        # Add conditional guard around CACHE_LINE_SIZE
        sed -i '/^#define.*CACHE_LINE_SIZE.*CACHE_LINE_SHIFT/i\
/* Unikraft CACHE_LINE_SIZE compatibility */\
#ifndef CACHE_LINE_SIZE' "$TARGET_INCLUDE_DIR/machine/param.h"
        
        sed -i '/^#define.*CACHE_LINE_SIZE.*CACHE_LINE_SHIFT/a\
#endif' "$TARGET_INCLUDE_DIR/machine/param.h"
        
        echo "  ✓ Added Unikraft CACHE_LINE_SIZE compatibility guard to machine/param.h"
    fi
fi

# Fix UIO_READ/UIO_WRITE enum conflict in sys/_uio.h
if [[ -f "$TARGET_INCLUDE_DIR/sys/_uio.h" ]]; then
    if grep -q "enum uio_rw" "$TARGET_INCLUDE_DIR/sys/_uio.h"; then
        # Comment out the enum to prevent redefinition conflicts
        sed -i '/enum uio_rw {/,/};/s/^/\/\//' "$TARGET_INCLUDE_DIR/sys/_uio.h"
        echo "  ✓ Commented out enum uio_rw in sys/_uio.h to prevent Unikraft conflicts"
    fi
fi

# Fix dirent64 structure - ensure it's properly defined
if [[ -f "$TARGET_INCLUDE_DIR/dirent.h" ]]; then
    if ! grep -q "struct dirent64" "$TARGET_INCLUDE_DIR/dirent.h"; then
        # Add dirent64 compatibility structure
        echo -e "
/* Linux compatibility */
#ifndef _DIRENT64_DEFINED
#define _DIRENT64_DEFINED
struct dirent64 {
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
};
#endif" >> "$TARGET_INCLUDE_DIR/dirent.h"
        echo "  ✓ Added dirent64 compatibility structure to dirent.h"
    fi
fi

# Fix FIONBIO definition issues in sys/filio.h
if [[ -f "$TARGET_INCLUDE_DIR/sys/filio.h" ]]; then
    if ! grep -q "FIONBIO.*int" "$TARGET_INCLUDE_DIR/sys/filio.h"; then
        # Ensure FIONBIO is properly defined with correct type
        sed -i '/^#define.*FIONBIO/c\
#define FIONBIO _IOW('''f''', 126, int)  /* set/clear non-blocking i/o */' "$TARGET_INCLUDE_DIR/sys/filio.h"
        echo "  ✓ Fixed FIONBIO definition type in sys/filio.h"
    fi
fi

# Comprehensive compatibility fixes for remaining macro issues
echo "  → Applying comprehensive compatibility fixes..."

# Fix additional attribute redefinitions in sys/cdefs.h
if [[ -f "$TARGET_INCLUDE_DIR/sys/cdefs.h" ]]; then
    if ! grep -q "Unikraft attribute compatibility" "$TARGET_INCLUDE_DIR/sys/cdefs.h"; then
        # Add guards around common attribute definitions that conflict with Unikraft
        sed -i '/^#define __unused.*__attribute__/i\
/* Unikraft attribute compatibility - prevent redefinition warnings */\
#ifndef __unused' "$TARGET_INCLUDE_DIR/sys/cdefs.h"
        
        sed -i '/^#define __unused.*__attribute__/a\
#endif' "$TARGET_INCLUDE_DIR/sys/cdefs.h"
        
        sed -i '/^#define __used.*__attribute__/i\
#ifndef __used' "$TARGET_INCLUDE_DIR/sys/cdefs.h"
        
        sed -i '/^#define __used.*__attribute__/a\
#endif' "$TARGET_INCLUDE_DIR/sys/cdefs.h"
        
        echo "    ✓ Added Unikraft attribute compatibility guards to sys/cdefs.h"
    fi
fi

# Fix compiler warnings by ensuring consistent types
if [[ -f "$TARGET_INCLUDE_DIR/sys/types.h" ]]; then
    if ! grep -q "Unikraft type compatibility" "$TARGET_INCLUDE_DIR/sys/types.h"; then
        # Add compatibility notes
        echo -e "\
/* Unikraft type compatibility notes added by sync script */" >> "$TARGET_INCLUDE_DIR/sys/types.h"
        echo "    ✓ Added type compatibility notes to sys/types.h"
    fi
fi

# Fix siginfo_t definition issues across signal.h and sys/wait.h
if [[ -f "$TARGET_INCLUDE_DIR/signal.h" ]]; then
    # Ensure siginfo_t is properly available in signal.h
    if ! grep -q "typedef.*siginfo_t" "$TARGET_INCLUDE_DIR/signal.h" && ! grep -q "_SIGINFO_T_DECLARED" "$TARGET_INCLUDE_DIR/signal.h"; then
        # Add forward declaration after the sys/signal.h include
        sed -i '/^#include <sys\/signal.h>/a\
\
/* Ensure siginfo_t is available for function prototypes */\
#ifndef _SIGINFO_T_DECLARED\
typedef struct __siginfo siginfo_t;\
#define _SIGINFO_T_DECLARED\
#endif' "$TARGET_INCLUDE_DIR/signal.h"
        echo "  ✓ Added siginfo_t typedef to signal.h"
    fi
fi

# Fix siginfo_t in sys/wait.h - ensure it uses siginfo_t instead of struct __siginfo
if [[ -f "$TARGET_INCLUDE_DIR/sys/wait.h" ]]; then
    if ! grep -q "siginfo_t" "$TARGET_INCLUDE_DIR/sys/wait.h"; then
        # Add siginfo_t typedef and fix function signatures
        sed -i '/^#include <sys\/types.h>/a\
\
/* Include signal definitions for siginfo_t */\
#include <sys/signal.h>' "$TARGET_INCLUDE_DIR/sys/wait.h"
        
        # Replace struct __siginfo with siginfo_t in function declarations
        sed -i 's/struct __siginfo \*/siginfo_t */g' "$TARGET_INCLUDE_DIR/sys/wait.h"
        
        echo "  ✓ Added siginfo_t support to sys/wait.h"
    fi
fi

# Also ensure a global siginfo_t header compatibility 
if [[ ! -f "$TARGET_INCLUDE_DIR/siginfo.h" ]]; then
    # Create a compatibility siginfo.h header
    cat > "$TARGET_INCLUDE_DIR/siginfo.h" << 'EOF'
#ifndef _SIGINFO_H_
#define _SIGINFO_H_

/* FreeBSD siginfo_t compatibility header */
#include <sys/signal.h>

#ifndef _SIGINFO_T_DECLARED
typedef struct __siginfo siginfo_t;
#define _SIGINFO_T_DECLARED
#endif

#endif /* _SIGINFO_H_ */
EOF
    echo "  ✓ Created siginfo.h compatibility header"
fi

# Fix missing _NSIG definition in sys/signal.h
if [[ -f "$TARGET_INCLUDE_DIR/sys/signal.h" ]]; then
    if ! grep -q "_NSIG" "$TARGET_INCLUDE_DIR/sys/signal.h"; then
        # Add _NSIG compatibility definition after NSIG
        sed -i '/^#define.*NSIG.*32/a\
#define\t_NSIG\t\tNSIG\t/* compatibility with Linux/musl */' "$TARGET_INCLUDE_DIR/sys/signal.h"
        echo "  ✓ Added _NSIG compatibility definition to sys/signal.h"
    fi
fi

# Ensure sys/stdint.h is available (needed by inttypes.h)
if [[ ! -f "$TARGET_INCLUDE_DIR/sys/stdint.h" ]] && [[ -f "$FREEBSD_SOURCE_DIR/sys/sys/stdint.h" ]]; then
    mkdir -p "$TARGET_INCLUDE_DIR/sys"
    cp "$FREEBSD_SOURCE_DIR/sys/sys/stdint.h" "$TARGET_INCLUDE_DIR/sys/stdint.h"
    echo "  ✓ Added sys/stdint.h for inttypes.h compatibility"
fi

# Fix missing termios constants and _IO macros
if [[ -f "$TARGET_INCLUDE_DIR/sys/termios.h" ]]; then
    if ! grep -q "VDISCARD\|VEOF\|VEOL" "$TARGET_INCLUDE_DIR/sys/termios.h"; then
        # Add missing termios constants at the end of the file
        cat >> "$TARGET_INCLUDE_DIR/sys/termios.h" << 'EOF'

/* Additional termios constants for Unikraft compatibility */
#ifndef VEOF
#define VEOF        0   /* EOF character */
#endif
#ifndef VEOL
#define VEOL        1   /* EOL character */  
#endif
#ifndef VDISCARD
#define VDISCARD    15  /* DISCARD character */
#endif
#ifndef VMIN
#define VMIN        16  /* MIN value (minimum chars) */
#endif
#ifndef VTIME
#define VTIME       17  /* TIME value (timeout) */
#endif

/* Terminal control constants */
#ifndef TCSBRK
#define TCSBRK      0   /* send break */
#endif
#ifndef TCSBRKP  
#define TCSBRKP     1   /* send break with duration */
#endif
#ifndef TCFLSH
#define TCFLSH      2   /* flush */
#endif
#ifndef TCDRAIN
#define TCDRAIN     3   /* drain */
#endif

EOF
        echo "  ✓ Added missing termios constants to sys/termios.h"
    fi
fi

# Fix missing _IO macro definitions in sys/ioccom.h
if [[ -f "$TARGET_INCLUDE_DIR/sys/ioccom.h" ]]; then
    if ! grep -q "_IO.*('t'," "$TARGET_INCLUDE_DIR/sys/ioccom.h"; then
        # Add common _IO definitions used by Unikraft
        cat >> "$TARGET_INCLUDE_DIR/sys/ioccom.h" << 'EOF'

/* Common ioctl definitions for Unikraft compatibility */
#ifndef TCGETS
#define TCGETS      _IOR('t', 19, struct termios)
#endif
#ifndef TCSETS  
#define TCSETS      _IOW('t', 20, struct termios)
#endif
#ifndef TCSETSW
#define TCSETSW     _IOW('t', 21, struct termios)  
#endif
#ifndef TCSETSF
#define TCSETSF     _IOW('t', 22, struct termios)
#endif
#ifndef TIOCGWINSZ
#define TIOCGWINSZ  _IOR('t', 104, struct winsize)
#endif
#ifndef TIOCSWINSZ
#define TIOCSWINSZ  _IOW('t', 103, struct winsize)
#endif
#ifndef TIOCSTI
#define TIOCSTI     _IOW('t', 114, char)
#endif
#ifndef TIOCSPGRP
#define TIOCSPGRP   _IOW('t', 118, int)
#endif
#ifndef TCSBRK
#define TCSBRK      _IO('t', 123)       /* send break */
#endif
#ifndef TCSBRKP
#define TCSBRKP     _IOW('t', 124, int) /* send break with arg */
#endif
#ifndef TCFLSH
#define TCFLSH      _IOW('t', 125, int) /* flush */
#endif
#ifndef TCDRAIN
#define TCDRAIN     _IO('t', 126)       /* drain */
#endif

EOF
        echo "  ✓ Added missing _IO macro definitions to sys/ioccom.h"
    fi
fi

# Fix missing access constants (R_OK, W_OK, X_OK, F_OK) - add to multiple headers for visibility
if [[ -f "$TARGET_INCLUDE_DIR/unistd.h" ]]; then
    if ! grep -q "R_OK.*W_OK.*X_OK.*F_OK" "$TARGET_INCLUDE_DIR/unistd.h"; then
        # Add access constants after sys/unistd.h include
        sed -i '/^#include <sys\/unistd.h>/a\
\
/* File access constants for access() function */\
#ifndef R_OK\
#define R_OK    4   /* Test for read permission */\
#define W_OK    2   /* Test for write permission */  \
#define X_OK    1   /* Test for execute permission */\
#define F_OK    0   /* Test for existence */\
#endif' "$TARGET_INCLUDE_DIR/unistd.h"
        echo "  ✓ Added access constants (R_OK, W_OK, X_OK, F_OK) to unistd.h"
    fi
fi

# Also add access constants to sys/unistd.h for broader visibility
if [[ -f "$TARGET_INCLUDE_DIR/sys/unistd.h" ]]; then
    if ! grep -q "R_OK.*W_OK.*X_OK.*F_OK" "$TARGET_INCLUDE_DIR/sys/unistd.h"; then
        cat >> "$TARGET_INCLUDE_DIR/sys/unistd.h" << 'EOF'

/* File access constants - global visibility */
#ifndef R_OK
#define R_OK    4   /* Test for read permission */
#define W_OK    2   /* Test for write permission */  
#define X_OK    1   /* Test for execute permission */
#define F_OK    0   /* Test for existence */
#endif

EOF
        echo "  ✓ Added access constants to sys/unistd.h for global visibility"
    fi
fi

# Fix missing O_LARGEFILE and O_TMPFILE in fcntl headers
if [[ -f "$TARGET_INCLUDE_DIR/sys/fcntl.h" ]]; then
    if ! grep -q "O_LARGEFILE\|O_TMPFILE" "$TARGET_INCLUDE_DIR/sys/fcntl.h"; then
        # Add missing file open flags
        cat >> "$TARGET_INCLUDE_DIR/sys/fcntl.h" << 'EOF'

/* Additional file open flags for Linux compatibility */
#ifndef O_LARGEFILE
#define O_LARGEFILE 0x00100000  /* Large file support */
#endif
#ifndef O_TMPFILE
#define O_TMPFILE   0x01000000  /* Temporary file */
#endif

/* File access constants - also in fcntl.h for broad visibility */
#ifndef R_OK
#define R_OK    4   /* Test for read permission */
#define W_OK    2   /* Test for write permission */  
#define X_OK    1   /* Test for execute permission */
#define F_OK    0   /* Test for existence */
#endif

EOF
        echo "  ✓ Added O_LARGEFILE and O_TMPFILE to sys/fcntl.h"
    fi
fi

# Also add access constants to root fcntl.h if it exists
if [[ -f "$TARGET_INCLUDE_DIR/fcntl.h" ]]; then
    if ! grep -q "R_OK.*W_OK.*X_OK.*F_OK" "$TARGET_INCLUDE_DIR/fcntl.h"; then
        cat >> "$TARGET_INCLUDE_DIR/fcntl.h" << 'EOF'

/* File access constants */
#ifndef R_OK
#define R_OK    4   /* Test for read permission */
#define W_OK    2   /* Test for write permission */  
#define X_OK    1   /* Test for execute permission */
#define F_OK    0   /* Test for existence */
#endif

EOF
        echo "  ✓ Added access constants to fcntl.h"
    fi
fi


# Fix struct statfs missing f_frsize and f_namelen members
if [[ -f "$TARGET_INCLUDE_DIR/sys/mount.h" ]]; then
    if ! grep -q "f_frsize" "$TARGET_INCLUDE_DIR/sys/mount.h"; then
        # Find the statfs structure and add f_frsize after f_bsize
        if grep -q "struct statfs" "$TARGET_INCLUDE_DIR/sys/mount.h"; then
            sed -i '/f_bsize;/a\
\tfsblkcnt_t f_frsize;    /* fundamental file system block size */' "$TARGET_INCLUDE_DIR/sys/mount.h"
            echo "  ✓ Added f_frsize member to struct statfs in sys/mount.h"
        fi
    fi
    
    # Add f_namelen as alias for f_namemax for Linux compatibility
    if ! grep -q "f_namelen" "$TARGET_INCLUDE_DIR/sys/mount.h"; then
        if grep -q "f_namemax" "$TARGET_INCLUDE_DIR/sys/mount.h"; then
            # Add f_namelen right after f_namemax
            sed -i '/f_namemax;/a\
\tlong f_namelen;         /* alias for f_namemax - Linux compatibility */' "$TARGET_INCLUDE_DIR/sys/mount.h"
            echo "  ✓ Added f_namelen member to struct statfs in sys/mount.h"
        fi
    fi
fi


# Also fix in sys/statvfs.h if it exists
if [[ -f "$TARGET_INCLUDE_DIR/sys/statvfs.h" ]]; then
    if ! grep -q "f_frsize.*Size of fragments" "$TARGET_INCLUDE_DIR/sys/statvfs.h"; then
        # Ensure f_frsize is properly defined in statvfs
        sed -i 's/unsigned long.*f_frsize.*/unsigned long\tf_frsize;\t\/* Size of fragments - Unikraft compatible *\//' "$TARGET_INCLUDE_DIR/sys/statvfs.h"
        echo "  ✓ Enhanced f_frsize definition in sys/statvfs.h"
    fi
fi

# Fix missing SEEK constants in multiple headers
for header in "stdio.h" "unistd.h" "sys/unistd.h"; do
    if [[ -f "$TARGET_INCLUDE_DIR/$header" ]]; then
        if ! grep -q "SEEK_SET.*SEEK_CUR.*SEEK_END" "$TARGET_INCLUDE_DIR/$header"; then
            # Add SEEK constants
            cat >> "$TARGET_INCLUDE_DIR/$header" << 'EOF'

/* SEEK constants for file positioning */
#ifndef SEEK_SET
#define SEEK_SET    0   /* set file offset to offset */
#define SEEK_CUR    1   /* set file offset to current plus offset */  
#define SEEK_END    2   /* set file offset to EOF plus offset */
#endif

EOF
            echo "  ✓ Added SEEK constants to $header"
        fi
    fi
done

# Fix missing PATH_MAX and related constants
if [[ -f "$TARGET_INCLUDE_DIR/sys/param.h" ]]; then
    if ! grep -q "PATH_MAX.*MAXPATHLEN" "$TARGET_INCLUDE_DIR/sys/param.h"; then
        # Ensure PATH_MAX is properly defined
        sed -i '/^#define.*MAXPATHLEN.*PATH_MAX/a\
#ifndef PATH_MAX\
#define PATH_MAX    1024    /* Maximum path length */\
#endif' "$TARGET_INCLUDE_DIR/sys/param.h"
        echo "  ✓ Added PATH_MAX definition to sys/param.h"
    fi
fi

# Fix missing NAME_MAX constant  
if [[ -f "$TARGET_INCLUDE_DIR/limits.h" ]]; then
    if ! grep -q "NAME_MAX" "$TARGET_INCLUDE_DIR/limits.h"; then
        # Add NAME_MAX definition
        cat >> "$TARGET_INCLUDE_DIR/limits.h" << 'EOF'

/* Additional system limits for Unikraft compatibility */
#ifndef NAME_MAX
#define NAME_MAX    255     /* Maximum filename length */
#endif
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX   64  /* Maximum hostname length */
#endif

EOF
        echo "  ✓ Added NAME_MAX and HOST_NAME_MAX to limits.h"
    fi
fi

# Fix missing terminal constants in termios.h (if it exists at root level)
if [[ -f "$TARGET_INCLUDE_DIR/termios.h" ]]; then
    if ! grep -q "NCCS\|_POSIX_VDISABLE" "$TARGET_INCLUDE_DIR/termios.h"; then
        # Add essential termios constants
        cat >> "$TARGET_INCLUDE_DIR/termios.h" << 'EOF'

/* Essential termios constants for Unikraft compatibility */
#ifndef NCCS
#define NCCS        20      /* Number of control characters */
#endif
#ifndef _POSIX_VDISABLE  
#define _POSIX_VDISABLE 0   /* Disabled control character */
#endif

/* Additional terminal constants */
#ifndef VINTR
#define VINTR       0   /* Interrupt character */
#endif
#ifndef VQUIT
#define VQUIT       1   /* Quit character */
#endif
#ifndef VERASE
#define VERASE      2   /* Erase character */
#endif
#ifndef VKILL
#define VKILL       3   /* Kill character */
#endif
#ifndef VSTART
#define VSTART      8   /* Start character */
#endif
#ifndef VSTOP
#define VSTOP       9   /* Stop character */
#endif
#ifndef VSUSP
#define VSUSP       10  /* Suspend character */
#endif

EOF
        echo "  ✓ Added NCCS and _POSIX_VDISABLE to termios.h"
    fi
fi

# Fix missing struct winsize if needed
if [[ -f "$TARGET_INCLUDE_DIR/sys/ttycom.h" ]]; then
    if ! grep -q "struct winsize" "$TARGET_INCLUDE_DIR/sys/ttycom.h"; then
        # Add winsize structure
        cat >> "$TARGET_INCLUDE_DIR/sys/ttycom.h" << 'EOF'

/* Window size structure for terminal I/O */
#ifndef _SYS_WINSIZE_H_
#define _SYS_WINSIZE_H_
struct winsize {
    unsigned short ws_row;      /* rows, in characters */
    unsigned short ws_col;      /* columns, in characters */ 
    unsigned short ws_xpixel;   /* horizontal size, pixels */
    unsigned short ws_ypixel;   /* vertical size, pixels */
};
#endif

EOF
        echo "  ✓ Added struct winsize to sys/ttycom.h"
    fi
fi


# Fix missing quotes in _IO macros in sys/ttycom.h (TCSBRK and TCSBRKP)
if [[ -f "$TARGET_INCLUDE_DIR/sys/ttycom.h" ]]; then
    if grep -q "_IO(t," "$TARGET_INCLUDE_DIR/sys/ttycom.h"; then
        # Fix the unquoted 't' in _IO macros - specifically TCSBRK and TCSBRKP
        sed -i 's/#define TCSBRK   _IO(t, 124)/#define TCSBRK   _IO('\''t'\'', 124)/' "$TARGET_INCLUDE_DIR/sys/ttycom.h"
        sed -i 's/#define TCSBRKP  _IO(t, 125)/#define TCSBRKP  _IO('\''t'\'', 125)/' "$TARGET_INCLUDE_DIR/sys/ttycom.h"
        echo "  ✓ Fixed _IO macro quotes in sys/ttycom.h (TCSBRK, TCSBRKP)"
    fi
fi

# Fix missing bsm/audit.h include in sys/ucred.h
if [[ -f "$TARGET_INCLUDE_DIR/sys/ucred.h" ]]; then
    if grep -q "#include <bsm/audit.h>" "$TARGET_INCLUDE_DIR/sys/ucred.h"; then
        # Comment out the problematic include since we don't have BSM audit support
        sed -i 's/#include <bsm\/audit.h>/\/\* #include <bsm\/audit.h> \*\/ \/\* BSM audit not available *\//' "$TARGET_INCLUDE_DIR/sys/ucred.h"
        echo "  ✓ Commented out bsm/audit.h include in sys/ucred.h"
    fi
fi


# Create sys/statfs.h compatibility header (statfs is defined in sys/mount.h in FreeBSD)
if [[ ! -f "$TARGET_INCLUDE_DIR/sys/statfs.h" ]]; then
    cat > "$TARGET_INCLUDE_DIR/sys/statfs.h" << 'EOF'
#ifndef _SYS_STATFS_H_
#define _SYS_STATFS_H_

/* FreeBSD statfs compatibility header
 * In FreeBSD, statfs functionality is in sys/mount.h
 */
#include <sys/mount.h>

#endif /* _SYS_STATFS_H_ */
EOF
    echo "  ✓ Created sys/statfs.h compatibility header"
fi

# Fix missing termios/ioctl constants
if [[ -f "$TARGET_INCLUDE_DIR/sys/ttycom.h" ]]; then
    if ! grep -q "TCXONC\|TIOCINQ\|TCFLSH\|TIOCGEXCL" "$TARGET_INCLUDE_DIR/sys/ttycom.h"; then
        # Add missing termios constants after existing definitions
        cat >> "$TARGET_INCLUDE_DIR/sys/ttycom.h" << 'EOF'

/* Additional termios/ioctl compatibility constants for Unikraft */
#ifndef TCXONC
#define TCXONC     _IO('t', 30)    /* start/stop control */
#endif
#ifndef TIOCINQ
#define TIOCINQ    FIONREAD        /* bytes in input queue */
#endif
#ifndef TCFLSH
#define TCFLSH     _IO('t', 16)    /* flush input/output queues */
#endif
#ifndef TIOCGEXCL
#define TIOCGEXCL  _IOR('t', 65, int) /* get exclusive use of tty (unique value) */
#endif
EOF
        echo "  ✓ Added missing termios constants (TCXONC, TIOCINQ, TCFLSH, TIOCGEXCL) to sys/ttycom.h"
    fi
fi

# Fix termios.h to properly include the termios constants
if [[ -f "$TARGET_INCLUDE_DIR/termios.h" ]]; then
    # Replace the simple wrapper with one that includes sys/_termios.h
    cat > "$TARGET_INCLUDE_DIR/termios.h" << 'EOF'
/*
 * FreeBSD termios.h wrapper for Unikraft
 */
#ifndef _TERMIOS_H_
#define _TERMIOS_H_

#include <sys/types.h>
#include <sys/_termios.h>
#include <sys/termios.h>

#endif /* _TERMIOS_H_ */
EOF
    echo "  ✓ Updated termios.h to include sys/_termios.h constants"
fi

# Add missing Linux-compatibility termios constants
if [[ -f "$TARGET_INCLUDE_DIR/sys/_termios.h" ]]; then
    if ! grep -q "VSWTC" "$TARGET_INCLUDE_DIR/sys/_termios.h"; then
        # Add Linux-specific constants that FreeBSD doesn't have
        # Note: VSWTC is unused in FreeBSD, assign it to an unused slot or make it a no-op
        echo -e "
/* Linux compatibility constants not in FreeBSD */
/* Note: VSWTC maps to existing VSTATUS to avoid array bounds issues */
#define	VSWTC		VSTATUS	/* Linux switch char (maps to VSTATUS) */
" >> "$TARGET_INCLUDE_DIR/sys/_termios.h"
        echo "  ✓ Added Linux-compatibility constants to sys/_termios.h"
    fi
fi

# Add Linux-compatibility constants to sys/ttycom.h
if [[ -f "$TARGET_INCLUDE_DIR/sys/ttycom.h" ]]; then
    if ! grep -q "TCGETS" "$TARGET_INCLUDE_DIR/sys/ttycom.h"; then
        # Add Linux termios ioctl compatibility 
        sed -i '/#include <sys\/ioccom.h>/a\
\
/* Linux compatibility - map Linux termios ioctls with unique values for switch statements */\
#ifndef TCGETS\
#define TCGETS   TIOCGETA    /* get termios struct */\
#define TCSETS   TIOCSETA    /* set termios struct */\
#define TCSETSW  TIOCSETAW   /* drain output, set */\
#define TCSETSF  TIOCSETAF   /* drain output, flush input, set */\
#define TCSBRK   _IO('''t''', 124)   /* send break (unique value) */\
#define TCSBRKP  _IO('''t''', 125)   /* send break (non-standard, unique) */\
#define TIOCINQ  FIONREAD    /* get # bytes available for reading */\
#define TCFLSH   TIOCFLUSH   /* flush terminal buffers */\
#define TCXONC   TIOCSTART   /* start/stop output (simplified mapping) */\
/* Fixed duplicate TIOCGEXCL definition with unique value */\
#endif' "$TARGET_INCLUDE_DIR/sys/ttycom.h"
        echo "  ✓ Added Linux termios ioctl compatibility to sys/ttycom.h"
    fi
fi

echo "✓ Header synchronization complete!"
echo ""
echo "Headers synced to: $TARGET_INCLUDE_DIR"
echo "Documentation updated: $FREEBSD_LIBC_ROOT/docs/HEADERS.md"
echo "Compatibility header: $TARGET_INCLUDE_DIR/unikraft_compat.h"
