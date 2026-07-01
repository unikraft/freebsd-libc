/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for nolibc's unistd.h. Adds POSIX declarations that nolibc
 * does not ship but SQLite references (access, getuid/geteuid, getpid,
 * fchown, readlink, ...). Most of these are stubbed in glue/stubs.c.
 */
#ifndef _UNISTD_FREEBSDPORT_OVERLAY_H_
#define _UNISTD_FREEBSDPORT_OVERLAY_H_

#include_next <unistd.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef R_OK
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#endif

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#endif

int   access(const char *path, int mode);

/*
 * vfscore's getcwd is registered through UK_SYSCALL_R_DEFINE with an
 * ssize_t return type (Linux syscall convention), not the POSIX char *.
 * Match that signature here so vfscore/main.c and SQLite's sqlite3.c
 * see the same declaration. SQLite casts via a function-pointer table
 * at use sites so the type difference at the call site is benign for
 * ABI purposes (both ssize_t and char * fit in %rax on x86_64).
 *
 * At runtime, getcwd() is only invoked by SQLite when resolving relative
 * paths; the simple in-memory / ramfs SQLite tests in this project do
 * not exercise it. Replace with a real POSIX wrapper if you intend to
 * use SQLite with on-disk databases.
 */
ssize_t getcwd(char *buf, size_t size);
int     ftruncate(int fd, off_t length);
int     truncate(const char *path, off_t length);
int   fchown(int fd, uid_t owner, gid_t group);
int   chown(const char *path, uid_t owner, gid_t group);
int   lchown(const char *path, uid_t owner, gid_t group);
uid_t getuid(void);
uid_t geteuid(void);
gid_t getgid(void);
gid_t getegid(void);
pid_t getpid(void);
pid_t getppid(void);
int   isatty(int fd);
int   link(const char *oldpath, const char *newpath);
int   symlink(const char *target, const char *linkpath);
ssize_t readlink(const char *path, char *buf, size_t bufsize);
int   unlink(const char *path);
int   rmdir(const char *path);
int   chdir(const char *path);
int   fchdir(int fd);
int   pipe(int pipefd[2]);
int   dup(int oldfd);
int   dup2(int oldfd, int newfd);
unsigned int alarm(unsigned int seconds);
int   pause(void);
int   fsync(int fd);
int   fdatasync(int fd);

/* gen/glob.c uses getlogin() for bare-~ expansion; glue stub returns NULL
 * (a unikernel has no login name), so ~ degrades to no expansion. */
char *getlogin(void);

#ifdef __cplusplus
}
#endif

#endif /* _UNISTD_FREEBSDPORT_OVERLAY_H_ */
