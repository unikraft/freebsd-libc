/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Overlay for FreeBSD's <glob.h> (for gen/glob.c), trimmed for this port:
 * size_t from <stddef.h>, struct stat/dirent forward-declared, all GLOB_* flags
 * defined unconditionally, and the __BLOCKS__ branch kept but inert (no
 * -fblocks). glob walks the FS via vfscore; ~user expansion via getpwnam glue.
 */
#ifndef _GLOB_H_
#define _GLOB_H_

#include <sys/cdefs.h>
#include <stddef.h>		/* size_t */

struct stat;
struct dirent;

typedef struct {
	size_t gl_pathc;	/* Count of total paths so far. */
	size_t gl_matchc;	/* Count of paths matching pattern. */
	size_t gl_offs;		/* Reserved at beginning of gl_pathv. */
	int gl_flags;		/* Copy of flags parameter to glob. */
	char **gl_pathv;	/* List of paths matching pattern. */
				/* Copy of error callback parameter to glob. */
	union {
		int (*gl_errfunc)(const char *, int);
#ifdef __BLOCKS__
		int (^gl_errblk)(const char *, int);
#else
		void *gl_errblk;
#endif
	};

	/*
	 * Alternate filesystem access methods for glob; replacement
	 * versions of closedir(3), readdir(3), opendir(3), stat(2) and lstat(2).
	 */
	void (*gl_closedir)(void *);
	struct dirent *(*gl_readdir)(void *);
	void *(*gl_opendir)(const char *);
	int (*gl_lstat)(const char *, struct stat *);
	int (*gl_stat)(const char *, struct stat *);
} glob_t;

/* POSIX flags */
#define	GLOB_APPEND	0x0001	/* Append to output from previous call. */
#define	GLOB_DOOFFS	0x0002	/* Use gl_offs. */
#define	GLOB_ERR	0x0004	/* Return on error. */
#define	GLOB_MARK	0x0008	/* Append / to matching directories. */
#define	GLOB_NOCHECK	0x0010	/* Return pattern itself if nothing matches. */
#define	GLOB_NOSORT	0x0020	/* Don't sort. */
#define	GLOB_NOESCAPE	0x2000	/* Disable backslash escaping. */

/* Error values returned by glob(3) */
#define	GLOB_NOSPACE	(-1)	/* Malloc call failed. */
#define	GLOB_ABORTED	(-2)	/* Unignored error. */
#define	GLOB_NOMATCH	(-3)	/* No match and GLOB_NOCHECK was not set. */
#define	GLOB_NOSYS	(-4)	/* Obsolete: source compatibility only. */

/* BSD extensions / internal flags used by glob.c */
#define	GLOB_ALTDIRFUNC	0x0040	/* Use alternately specified directory funcs. */
#define	GLOB_BRACE	0x0080	/* Expand braces ala csh. */
#define	GLOB_MAGCHAR	0x0100	/* Pattern had globbing characters. */
#define	GLOB_NOMAGIC	0x0200	/* GLOB_NOCHECK without magic chars (csh). */
#define	GLOB_QUOTE	0x0400	/* Quote special chars with \. */
#define	GLOB_TILDE	0x0800	/* Expand tilde names from the passwd file. */
#define	GLOB_LIMIT	0x1000	/* limit number of returned paths */
#define	_GLOB_ERR_BLOCK	0x08000000 /* (internal) error callback is a block */

/* source compatibility, these are the old names */
#define	GLOB_MAXPATH	GLOB_LIMIT
#define	GLOB_ABEND	GLOB_ABORTED

__BEGIN_DECLS
int	glob(const char * __restrict, int,
	    int (*)(const char *, int), glob_t * __restrict);
void	globfree(glob_t *);
__END_DECLS

#endif /* !_GLOB_H_ */
