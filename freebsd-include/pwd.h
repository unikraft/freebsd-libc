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
