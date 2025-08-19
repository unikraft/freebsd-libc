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
