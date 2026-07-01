/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * stdio.h overlay - the header half of the FreeBSD stdio port. FILE is a viral
 * type: the whole image must agree on one struct FILE, so this header is the
 * global switch, gated by CONFIG_LIBFREEBSD_LIBC_STDIO:
 *   unset -> FILE == nolibc's _nolibc_file, served by nolibc stdio (default).
 *   set   -> FILE == FreeBSD's __sFILE, served by lib/libc/stdio.
 * The ON branch transcribes FreeBSD 15's <stdio.h> as a self-contained overlay
 * (rather than adopting FreeBSD's header tree); all stdio .c files include this.
 */
#ifndef _STDIO_FREEBSDPORT_OVERLAY_H_
#define _STDIO_FREEBSDPORT_OVERLAY_H_

/* The switch keys on a Kconfig symbol, which only becomes a C macro via
 * <uk/config.h> (not force-included). Pull it in first, or the #if below always
 * sees "undefined" and falls to the nolibc branch. */
#include <uk/config.h>

#if defined(CONFIG_LIBFREEBSD_LIBC_STDIO)

/* ====================================================================== *
 *  FreeBSD __sFILE stdio (port in progress)                              *
 * ====================================================================== */
#include <sys/cdefs.h>
#include <stddef.h>	/* size_t, NULL */
#include <stdarg.h>	/* va_list */

__BEGIN_DECLS

#ifndef _OFF_T_DECLARED
typedef long		off_t;
#define _OFF_T_DECLARED
#endif
#ifndef _SSIZE_T_DECLARED
typedef long		ssize_t;
#define _SSIZE_T_DECLARED
#endif
typedef off_t		fpos_t;
typedef long		off64_t;

/* FreeBSD spells the variadic list __va_list internally. */
#ifndef _VA_LIST_DECLARED
typedef __builtin_va_list	__va_list;
#define _VA_LIST_DECLARED
#endif

/* mbstate_t: see the note in <wchar.h> - defined identically there, behind
 * the same guard, so include order does not matter. __sFILE embeds it. */
#ifndef _MBSTATE_T_DECLARED
typedef union {
	char		__mbstate8[128];
	long long	_mbstateL;
} __mbstate_t;
typedef __mbstate_t mbstate_t;
#define _MBSTATE_T_DECLARED
#endif

#define _FSTDIO			/* Define for new stdio with functions. */

/* stdio buffers */
struct __sbuf {
	unsigned char *_base;
	int	_size;
};

struct __sFILE {
	unsigned char *_p;	/* current position in (some) buffer */
	int	_r;		/* read space left for getc() */
	int	_w;		/* write space left for putc() */
	short	_flags;		/* flags, below; this FILE is free if 0 */
	short	_file;		/* fileno, if Unix descriptor, else -1 */
	struct	__sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
	int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

	/* operations */
	void	*_cookie;	/* cookie passed to io functions */
	int	(* _Nullable _close)(void *);
	int	(* _Nullable _read)(void *, char *, int);
	fpos_t	(* _Nullable _seek)(void *, fpos_t, int);
	int	(* _Nullable _write)(void *, const char *, int);

	/* separate buffer for long sequences of ungetc() */
	struct	__sbuf _ub;	/* ungetc buffer */
	unsigned char	*_up;	/* saved _p when _p is doing ungetc data */
	int	_ur;		/* saved _r when _r is counting ungetc data */

	/* tricks to meet minimum requirements even when malloc() fails */
	unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
	unsigned char _nbuf[1];	/* guarantee a getc() buffer */

	/* separate buffer for fgetln() when line crosses buffer boundary */
	struct	__sbuf _lb;	/* buffer for fgetln() */

	/* Unix stdio files get aligned to block boundaries on fseek() */
	int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
	fpos_t	_offset;	/* current lseek offset */

	struct pthread_mutex *_fl_mutex;	/* used for MT-safety */
	struct pthread *_fl_owner;	/* current owner */
	int	_fl_count;	/* recursive lock count */
	int	_orientation;	/* orientation for fwide() */
	__mbstate_t _mbstate;	/* multibyte conversion state */
	int	_flags2;	/* additional flags */
};
#ifndef _STDFILE_DECLARED
#define _STDFILE_DECLARED
typedef struct __sFILE FILE;
#endif
#ifndef _STDSTREAM_DECLARED
extern FILE *__stdinp;
extern FILE *__stdoutp;
extern FILE *__stderrp;
#define _STDSTREAM_DECLARED
#endif

#define	__SLBF	0x0001		/* line buffered */
#define	__SNBF	0x0002		/* unbuffered */
#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
#define	__SRW	0x0010		/* open for reading & writing */
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
#define	__SMBF	0x0080		/* _bf._base is from malloc */
#define	__SAPP	0x0100		/* fdopen()ed in append mode */
#define	__SSTR	0x0200		/* this is an sprintf/snprintf string */
#define	__SOPT	0x0400		/* do fseek() optimization */
#define	__SNPT	0x0800		/* do not do fseek() optimization */
#define	__SOFF	0x1000		/* set iff _offset is in fact correct */
#define	__SMOD	0x2000		/* true => fgetln modified _p text */
#define	__SALC	0x4000		/* allocate string space dynamically */
#define	__SIGN	0x8000		/* ignore this file in _fwalk */

#define	__S2OAP	0x0001		/* O_APPEND mode is set */

#define	_IOFBF	0		/* setvbuf should set fully buffered */
#define	_IOLBF	1		/* setvbuf should set line buffered */
#define	_IONBF	2		/* setvbuf should set unbuffered */

#define	BUFSIZ	1024
#define	EOF	(-1)

#ifndef FOPEN_MAX
#define	FOPEN_MAX	20
#endif
#define	FILENAME_MAX	1024
#define	P_tmpdir	"/tmp/"
#define	L_tmpnam	1024
#define	TMP_MAX		308915776

#ifndef SEEK_SET
#define	SEEK_SET	0
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1
#endif
#ifndef SEEK_END
#define	SEEK_END	2
#endif

#define	stdin	__stdinp
#define	stdout	__stdoutp
#define	stderr	__stderrp

/*
 * ANSI C / POSIX functions (transcribed from FreeBSD 15; the full surface is
 * declared unconditionally).
 */
void	 clearerr(FILE *);
int	 fclose(FILE *);
int	 feof(FILE *);
int	 ferror(FILE *);
int	 fflush(FILE *);
int	 fgetc(FILE *);
int	 fgetpos(FILE * __restrict, fpos_t * __restrict);
char	*(fgets)(char * __restrict, int, FILE * __restrict);
FILE	*fopen(const char * __restrict, const char * __restrict);
int	 fprintf(FILE * __restrict, const char * __restrict, ...) __printflike(2, 3);
int	 fputc(int, FILE *);
int	 fputs(const char * __restrict, FILE * __restrict);
size_t	 fread(void * __restrict, size_t, size_t, FILE * __restrict);
FILE	*freopen(const char * __restrict, const char * __restrict, FILE * __restrict);
int	 fscanf(FILE * __restrict, const char * __restrict, ...) __scanflike(2, 3);
int	 fseek(FILE *, long, int);
int	 fsetpos(FILE *, const fpos_t *);
long	 ftell(FILE *);
size_t	 fwrite(const void * __restrict, size_t, size_t, FILE * __restrict);
int	 getc(FILE *);
int	 getchar(void);
void	 perror(const char *);
int	 printf(const char * __restrict, ...) __printflike(1, 2);
int	 putc(int, FILE *);
int	 putchar(int);
int	 puts(const char *);
int	 remove(const char *);
int	 rename(const char *, const char *);
void	 rewind(FILE *);
int	 scanf(const char * __restrict, ...) __scanflike(1, 2);
void	 setbuf(FILE * __restrict, char * __restrict);
int	 setvbuf(FILE * __restrict, char * __restrict, int, size_t);
int	 (sprintf)(char * __restrict, const char * __restrict, ...) __printflike(2, 3);
int	 sscanf(const char * __restrict, const char * __restrict, ...) __scanflike(2, 3);
FILE	*tmpfile(void);
char	*tmpnam(char *);
int	 ungetc(int, FILE *);
int	 vfprintf(FILE * __restrict, const char * __restrict, __va_list) __printflike(2, 0);
int	 vprintf(const char * __restrict, __va_list) __printflike(1, 0);
int	 (vsprintf)(char * __restrict, const char * __restrict, __va_list) __printflike(2, 0);
int	 (snprintf)(char * __restrict, size_t, const char * __restrict, ...) __printflike(3, 4);
int	 (vsnprintf)(char * __restrict, size_t, const char * __restrict, __va_list) __printflike(3, 0);
int	 vfscanf(FILE * __restrict, const char * __restrict, __va_list) __scanflike(2, 0);
int	 vscanf(const char * __restrict, __va_list) __scanflike(1, 0);
int	 vsscanf(const char * __restrict, const char * __restrict, __va_list) __scanflike(2, 0);

char	*ctermid(char *);
FILE	*fdopen(int, const char *);
int	 fileno(FILE *);
int	 pclose(FILE *);
FILE	*popen(const char *, const char *);

int	 ftrylockfile(FILE *);
void	 flockfile(FILE *);
void	 funlockfile(FILE *);
int	 getc_unlocked(FILE *);
int	 getchar_unlocked(void);
int	 putc_unlocked(int, FILE *);
int	 putchar_unlocked(int);
void	 clearerr_unlocked(FILE *);
int	 feof_unlocked(FILE *);
int	 ferror_unlocked(FILE *);

int	 fseeko(FILE *, off_t, int);
off_t	 ftello(FILE *);

/* Purely-local (BSD) routines used by the stdio machinery / SQLite. */
int	 asprintf(char **, const char *, ...) __printflike(2, 3);
void	 fcloseall(void);
char	*fgetln(FILE *, size_t *);
int	 fpurge(FILE *);
void	 setbuffer(FILE *, char *, int);
int	 setlinebuf(FILE *);
int	 vasprintf(char **, const char *, __va_list) __printflike(2, 0);
ssize_t	 getdelim(char ** __restrict, size_t * __restrict, int, FILE * __restrict);
ssize_t	 getline(char ** __restrict, size_t * __restrict, FILE * __restrict);
int	 dprintf(int, const char * __restrict, ...) __printflike(2, 3);
int	 vdprintf(int, const char * __restrict, __va_list) __printflike(2, 0);

/* Stdio function-access interface (funopen / fopencookie). */
FILE	*funopen(const void *,
	    int (* _Nullable)(void *, char *, int),
	    int (* _Nullable)(void *, const char *, int),
	    fpos_t (* _Nullable)(void *, fpos_t, int),
	    int (* _Nullable)(void *));
#define	fropen(cookie, fn) funopen(cookie, fn, 0, 0, 0)
#define	fwopen(cookie, fn) funopen(cookie, 0, fn, 0, 0)

/*
 * Functions internal to the implementation.
 */
int	__srget(FILE *);
int	__swbuf(int, FILE *);

#define	__sgetc(p) (--(p)->_r < 0 ? __srget(p) : (int)(*(p)->_p++))
static __inline int __sputc(int _c, FILE *_p) {
	if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
		return (*_p->_p++ = _c);
	else
		return (__swbuf(_c, _p));
}

#ifndef __LIBC_ISTHREADED_DECLARED
#define __LIBC_ISTHREADED_DECLARED
extern int __isthreaded;	/* defined = 0 in glue/stubs.c */
#endif

#ifndef __cplusplus
#define	__sfeof(p)	(((p)->_flags & __SEOF) != 0)
#define	__sferror(p)	(((p)->_flags & __SERR) != 0)
#define	__sclearerr(p)	((void)((p)->_flags &= ~(__SERR|__SEOF)))
#define	__sfileno(p)	((p)->_file)

#define	feof(p)		(!__isthreaded ? __sfeof(p)    : (feof)(p))
#define	ferror(p)	(!__isthreaded ? __sferror(p)  : (ferror)(p))
#define	clearerr(p)	(!__isthreaded ? __sclearerr(p): (clearerr)(p))
#define	getc(fp)	(!__isthreaded ? __sgetc(fp)   : (getc)(fp))
#define	putc(x, fp)	(!__isthreaded ? __sputc(x, fp): (putc)(x, fp))
#define	getchar()	getc(stdin)
#define	putchar(x)	putc(x, stdout)

#define	clearerr_unlocked(p)	__sclearerr(p)
#define	feof_unlocked(p)	__sfeof(p)
#define	ferror_unlocked(p)	__sferror(p)
#define	fileno_unlocked(p)	__sfileno(p)
#define	fputc_unlocked(s, p)	__sputc(s, p)
#define	getc_unlocked(fp)	__sgetc(fp)
#define	putc_unlocked(x, fp)	__sputc(x, fp)
#define	getchar_unlocked()	getc_unlocked(stdin)
#define	putchar_unlocked(x)	putc_unlocked(x, stdout)
#endif /* __cplusplus */

__END_DECLS

#else /* !CONFIG_LIBFREEBSD_LIBC_STDIO */

/* ====================================================================== *
 *  Original overlay for nolibc's stdio.h (default, known-good path).     *
 *  Adds the buffer-mode constants and setvbuf/setbuf nolibc omits.       *
 * ====================================================================== */
#include_next <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _IOFBF
#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2
#endif

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

#ifndef L_tmpnam
#define L_tmpnam 64
#endif

#ifndef FILENAME_MAX
#define FILENAME_MAX 4096
#endif

#ifndef FOPEN_MAX
#define FOPEN_MAX 1024
#endif

#ifndef TMP_MAX
#define TMP_MAX 10000
#endif

/* nolibc's stdio implementation is unbuffered already; setvbuf is a no-op. */
static inline int setvbuf(FILE *stream __attribute__((unused)),
			  char *buf       __attribute__((unused)),
			  int mode        __attribute__((unused)),
			  size_t size     __attribute__((unused)))
{
	return 0;
}

static inline void setbuf(FILE *stream __attribute__((unused)),
			  char *buf      __attribute__((unused)))
{
}

#ifdef __cplusplus
}
#endif

#endif /* CONFIG_LIBFREEBSD_LIBC_STDIO */

#endif /* _STDIO_FREEBSDPORT_OVERLAY_H_ */
