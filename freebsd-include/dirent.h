/*
 * FreeBSD dirent.h wrapper for Unikraft
 */
#ifndef _DIRENT_H_
#define _DIRENT_H_

#include <sys/dirent.h>

/* DIR type definition for directory operations */
#ifndef _DIR_DEFINED
#define _DIR_DEFINED

/* Forward declaration - actual definition in Unikraft vfscore */
struct __dirstream;
typedef struct __dirstream DIR;

#endif

/* Directory operation function declarations */
DIR *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
int closedir(DIR *dirp);
void rewinddir(DIR *dirp);
long telldir(DIR *dirp);
void seekdir(DIR *dirp, long loc);
int scandir(const char *dirname, struct dirent ***namelist,
           int (*select)(const struct dirent *),
           int (*compar)(const struct dirent **, const struct dirent **));
int alphasort(const struct dirent **a, const struct dirent **b);

/* 64-bit directory functions - provide forward declarations to prevent implicit declarations */
struct dirent64 *readdir64(DIR *dirp);
int readdir64_r(DIR *dirp, struct dirent64 *entry, struct dirent64 **result);

#endif /* _DIRENT_H_ */

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
#endif
