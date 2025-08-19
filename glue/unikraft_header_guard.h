/*
 * Unikraft Header Guard for FreeBSD libc Integration
 * 
 * This header prevents FreeBSD system headers from overriding Unikraft 
 * internal headers when compiling Unikraft core libraries.
 * 
 * Strategy: Use compiler-specific detection to determine if we're compiling
 * Unikraft internal code vs application code, and selectively prevent
 * problematic FreeBSD headers.
 */

#ifndef _UNIKRAFT_HEADER_GUARD_H
#define _UNIKRAFT_HEADER_GUARD_H

/* Detect if we're compiling Unikraft internal libraries */
#if defined(__UNIKRAFT__) && defined(FREEBSD_LIBC_INTEGRATION)

/* Check if we're in a Unikraft library source file */
#if defined(__FILE__) && (strstr(__FILE__, "/repos/unikraft/lib/") != NULL || \
                          strstr(__FILE__, "/repos/unikraft/plat/") != NULL || \
                          strstr(__FILE__, "/repos/unikraft/arch/") != NULL)

/* We're in Unikraft internal code - prevent problematic FreeBSD headers */
#define _UNIKRAFT_INTERNAL_BUILD 1

/* Prevent FreeBSD sys/mount.h from being included */
#ifdef _SYS_MOUNT_H_
#define _FREEBSD_MOUNT_H_INCLUDED_ALREADY
#endif

#ifndef _SYS_MOUNT_H_
#define _SYS_MOUNT_H_ 1  /* Mark as included to prevent FreeBSD version */

/* Provide Unikraft-compatible mount declarations */
#ifdef __cplusplus
extern "C" {
#endif

/* Use Unikraft/Linux mount signature for internal code */
int mount(const char *source, const char *target, const char *filesystemtype,
          unsigned long mountflags, const void *data);
int umount(const char *target);
int umount2(const char *target, int flags);

/* Mount flags (Linux/Unikraft compatible) */
#define MS_RDONLY      1
#define MS_NOSUID      2
#define MS_NODEV       4
#define MS_NOEXEC      8
#define MS_SYNCHRONOUS 16
#define MS_REMOUNT     32
#define MS_BIND        4096
#define MS_MOVE        8192
#define MS_REC         16384

#define MNT_FORCE       0x00000001
#define MNT_DETACH      0x00000002
#define UMOUNT_NOFOLLOW 0x00000008

#ifdef __cplusplus
}
#endif

#endif /* _SYS_MOUNT_H_ guard */

#endif /* _UNIKRAFT_INTERNAL_BUILD check */

#endif /* __UNIKRAFT__ && FREEBSD_LIBC_INTEGRATION */

#endif /* _UNIKRAFT_HEADER_GUARD_H */