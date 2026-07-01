/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Glue stubs for libc functions nolibc doesn't provide and that are too
 * entangled to pull from FreeBSD directly (e.g. strtod needs gdtoa). Minimal
 * implementations, enough for SQLite to link and run.
 */

#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include <uk/assert.h>

void __assert_fail(const char *expr, const char *file,
		   unsigned int line, const char *func)
{
	fprintf(stderr, "Assertion failed: %s, function %s, file %s, line %u\n",
		expr, func, file, line);
	UK_CRASH("assertion failed: %s\n", expr);
}

/* FreeBSD reaches errno through *__error(); bridge it to nolibc's errno. */
int *__error(void)
{
	return &errno;
}

/* strtod/strtof/strtold now come from FreeBSD gdtoa (see Makefile.uk); only
 * atof below still bridges to strtod, which is exactly what atof is. */

/* Single-threaded: gdtoa's locks test this flag and never engage. */
int __isthreaded = 0;

/* We always run round-to-nearest, so FLT_ROUNDS == 1. */
int __flt_rounds(void)
{
	return 1;
}

/* Scratch buffer for inet_nsap_ntoa() when the caller passes NULL. Single-
 * threaded, so a global is fine. (255*3 = max NSAP text.) */
char inet_nsap_ntoa_tmpbuf[255 * 3];

double atof(const char *nptr)
{
	return strtod(nptr, NULL);
}

/* getenv/setenv/unsetenv/putenv: the real ones come from posix-environ when
 * enabled, so gate these fallbacks out to avoid a multiple-definition error. */
#if !defined(CONFIG_LIBPOSIX_ENVIRON)
char *getenv(const char *name __attribute__((unused)))
{
	return NULL;
}

int setenv(const char *name __attribute__((unused)),
	   const char *value __attribute__((unused)),
	   int overwrite __attribute__((unused)))
{
	return 0;
}

int unsetenv(const char *name __attribute__((unused)))
{
	return 0;
}

int putenv(char *string __attribute__((unused)))
{
	return 0;
}
#endif /* !CONFIG_LIBPOSIX_ENVIRON */

/* SQLite calls system() for shell .system commands; stub it out. */
int system(const char *command __attribute__((unused)))
{
	return -1;
}

#if !defined(CONFIG_LIBFREEBSD_LIBC_STDIO)
/*
 * Minimal stdio extensions nolibc doesn't provide. Compiled only on the nolibc
 * stdio path; the FreeBSD stdio port supplies these for real.
 */
extern int fseek(FILE *stream, long offset, int whence);
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
extern int feof(FILE *stream);
extern int ferror(FILE *stream);

long ftell(FILE *stream)
{
	/* nolibc has no ftell; unsupported here. */
	(void)stream;
	return 0;
}

void rewind(FILE *stream)
{
	if (stream)
		fseek(stream, 0, 0 /* SEEK_SET */);
}

int fgetc(FILE *stream)
{
	unsigned char c;
	if (!stream)
		return -1;
	if (fread(&c, 1, 1, stream) != 1)
		return -1;
	return (int)c;
}

int getc(FILE *stream)            { return fgetc(stream); }
int getchar(void)                 { return fgetc(stdin); }

int putc(int c, FILE *stream)
{
	unsigned char b = (unsigned char)c;
	if (!stream)
		return -1;
	if (fwrite(&b, 1, 1, stream) != 1)
		return -1;
	return c;
}

char *fgets(char *s, int size, FILE *stream)
{
	int i = 0;
	int c;
	if (!s || size <= 0 || !stream)
		return NULL;
	while (i < size - 1) {
		c = fgetc(stream);
		if (c < 0) {
			if (i == 0)
				return NULL;
			break;
		}
		s[i++] = (char)c;
		if (c == '\n')
			break;
	}
	s[i] = '\0';
	return s;
}

int ungetc(int c __attribute__((unused)), FILE *stream __attribute__((unused)))
{
	return -1;
}

int fileno(FILE *stream __attribute__((unused)))
{
	/* nolibc doesn't expose the fd; -1 is safe here. */
	return -1;
}

#else  /* CONFIG_LIBFREEBSD_LIBC_STDIO */
/*
 * Platform glue for the FreeBSD stdio port. BSD stdio routes all I/O through
 * __sread/__swrite/__sseek/__sclose, which call these wrappers. fds 0-2 go to
 * the Unikraft console; everything else to the real syscalls.
 */
#include <unistd.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <wchar.h>
#include <fcntl.h>
#include <stdarg.h>
#include <uk/console.h>

ssize_t _write(int fd, const void *buf, size_t n)
{
	if (fd >= 0 && fd < 3)
		return (ssize_t)uk_console_out((const char *)buf, (__sz)n);
	return write(fd, buf, n);
}

/* No scatter/gather syscall wired up; fan _writev out over _write. */
ssize_t _writev(int fd, const struct iovec *iov, int iovcnt)
{
	ssize_t total = 0;
	for (int i = 0; i < iovcnt; i++) {
		if (iov[i].iov_len == 0)
			continue;
		ssize_t r = _write(fd, iov[i].iov_base, iov[i].iov_len);
		if (r < 0)
			return total ? total : r;
		total += r;
		if ((size_t)r < iov[i].iov_len)
			break;
	}
	return total;
}

ssize_t _read(int fd, void *buf, size_t n)
{
	return read(fd, buf, n);
}

int _close(int fd)
{
	return close(fd);
}

/* stdio opens backing files through _open; forward to open(). */
int _open(const char *path, int flags, ...)
{
	int mode = 0;
	if (flags & O_CREAT) {
		va_list ap;
		va_start(ap, flags);
		mode = va_arg(ap, int);
		va_end(ap);
	}
	return open(path, flags, mode);
}

/* Wide-char read helpers for vfscanf's %ls/%lc. Byte-oriented port, so these
 * report end-of-wide-input. */
wint_t __fgetwc_mbs(FILE *fp, mbstate_t *mbs, int *nread, void *locale)
{
	(void)fp; (void)mbs; (void)locale;
	if (nread)
		*nread = 0;
	return WEOF;
}

wint_t __ungetwc(wint_t wc, FILE *fp, void *locale)
{
	(void)wc; (void)fp; (void)locale;
	return WEOF;
}

/* wide-char -> single byte (C locale): identity for ASCII, else EOF. */
int wctob(wint_t c)
{
	return ((unsigned)c < 128) ? (int)c : EOF;
}

/* makebuf.c calls _fstat() only for st_blksize; return -1 so it uses BUFSIZ. */
int _fstat(int fd, struct stat *st)
{
	(void)fd;
	(void)st;
	return -1;
}

/* stdio flush-on-exit hook slot; wiring the exit path to it is a later phase. */
void (*__cleanup)(void);

/* Deprecated f_prealloc() sizes itself from this; never called in our build. */
int getdtablesize(void)
{
	return 1024;
}

/* We don't build xprintf; keep this 0 so vfprintf's extended path stays dead. */
int __use_xprintf = 0;

/* FreeBSD stdio buffers stdout, and a unikernel has no exit-time fflush, so a
 * buffered stdout could lose output at halt. Make it unbuffered at boot (this
 * constructor runs before main, while stdout is untouched). */
__attribute__((constructor))
static void __fbsdport_stdio_unbuffer(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
}

/* xprintf entry point, referenced under the dead __use_xprintf path; exists
 * only to satisfy the linker. */
int __xvprintf(FILE *fp, const char *fmt, __builtin_va_list ap)
{
	(void)fp; (void)fmt; (void)ap;
	return -1;
}

/* Locale-aware strerror_r for vfprintf's %m; format a generic message. */
char *__strerror_rl(int errnum, char *buf, size_t buflen, void *locale)
{
	(void)locale;
	if (buf && buflen)
		snprintf(buf, buflen, "Error %d", errnum);
	return buf;
}

/* C-locale wide<->byte conversion for vfprintf/vfscanf: one wchar_t = one byte. */
size_t wcrtomb(char *s, wchar_t wc, mbstate_t *ps)
{
	(void)ps;
	if (s)
		*s = (char)wc;
	return 1;
}

/* Reached only on the non-C-locale path (never taken); satisfies the linker. */
int __collate_range_cmp(int c1, int c2)
{
	return c1 - c2;
}

size_t wcsrtombs(char *dst, const wchar_t **src, size_t len, mbstate_t *ps)
{
	const wchar_t *s;
	size_t n = 0;
	(void)ps;
	if (!src || !*src)
		return 0;
	s = *src;
	while (!dst || n < len) {
		wchar_t wc = *s++;
		if (dst)
			dst[n] = (char)wc;
		if (wc == 0) {
			if (dst)
				*src = NULL;
			return n;
		}
		n++;
	}
	if (dst)
		*src = s;
	return n;
}

#endif /* CONFIG_LIBFREEBSD_LIBC_STDIO */

/*
 * gen/fnmatch.c support: C-locale wide-char + collation shims. One byte == one
 * char; the named classes map onto <ctype.h>; the collation helpers report
 * "nothing", so fnmatch falls back to plain character handling. Built
 * unconditionally (fnmatch is in the gen layer, independent of the stdio flag).
 */
#include <wchar.h>
#include <wctype.h>
#include <collate.h>

size_t mbrtowc(wchar_t * __restrict pwc, const char * __restrict s, size_t n,
	       mbstate_t * __restrict ps __attribute__((unused)))
{
	if (s == NULL)
		return 0;		/* C locale is stateless */
	if (n == 0)
		return (size_t)-2;	/* incomplete */
	if (pwc != NULL)
		*pwc = (wchar_t)(unsigned char)*s;
	return (*s == '\0') ? 0 : 1;
}

/* Named character classes, C locale; iswctype() dispatches to <wctype.h>. */
wctype_t wctype(const char *name)
{
	static const char * const names[] = {
		"alnum", "alpha", "blank", "cntrl", "digit", "graph",
		"lower", "print", "punct", "space", "upper", "xdigit"
	};
	for (size_t i = 0; i < sizeof(names) / sizeof(names[0]); i++)
		if (strcmp(name, names[i]) == 0)
			return (wctype_t)(i + 1);
	return (wctype_t)0;
}

int iswctype(wint_t wc, wctype_t desc)
{
	switch (desc) {
	case 1:  return iswalnum(wc);
	case 2:  return iswalpha(wc);
	case 3:  return iswblank(wc);
	case 4:  return iswcntrl(wc);
	case 5:  return iswdigit(wc);
	case 6:  return iswgraph(wc);
	case 7:  return iswlower(wc);
	case 8:  return iswprint(wc);
	case 9:  return iswpunct(wc);
	case 10: return iswspace(wc);
	case 11: return iswupper(wc);
	case 12: return iswxdigit(wc);
	default: return 0;
	}
}

/* No collation tables in the C locale: report "nothing here". */
size_t __collate_collating_symbol(wchar_t *result __attribute__((unused)),
		size_t rlen __attribute__((unused)),
		const char *src __attribute__((unused)),
		size_t slen __attribute__((unused)),
		mbstate_t *st __attribute__((unused)))
{
	return 0;		/* not a collating symbol */
}

int __collate_equiv_class(const char *src __attribute__((unused)),
		size_t slen __attribute__((unused)),
		mbstate_t *st __attribute__((unused)))
{
	return 0;		/* no equivalence class */
}

size_t __collate_equiv_match(int equiv __attribute__((unused)),
		wchar_t *result __attribute__((unused)),
		size_t rlen __attribute__((unused)),
		wchar_t start __attribute__((unused)),
		const char *src __attribute__((unused)),
		size_t slen __attribute__((unused)),
		mbstate_t *st __attribute__((unused)),
		size_t *lenp __attribute__((unused)))
{
	return 0;		/* never matches in C locale */
}

/* Reached only on the non-C-locale range path (never taken); linker only. */
int __wcollate_range_cmp(wchar_t c1, wchar_t c2)
{
	return (c1 > c2) - (c1 < c2);
}

/* Minimal C-locale rune table for regex/regcomp.c; only ->__encoding is read,
 * and only on a path this port never takes. */
static struct __fbsdport_runelocale __fbsdport_c_rune = { "NONE" };
struct __fbsdport_runelocale *_CurrentRuneLocale = &__fbsdport_c_rune;

/* popen/pclose/system/signal - meaningless in a unikernel; stub to fail. */
FILE *popen(const char *command __attribute__((unused)),
	    const char *mode    __attribute__((unused)))
{
	return NULL;
}

int pclose(FILE *stream __attribute__((unused)))
{
	return -1;
}

typedef void (*sighandler_t)(int);

sighandler_t signal(int signum __attribute__((unused)),
		    sighandler_t handler __attribute__((unused)))
{
	return (sighandler_t)0;
}

int raise(int sig __attribute__((unused)))
{
	return 0;
}

int kill(pid_t pid __attribute__((unused)), int sig __attribute__((unused)))
{
	return 0;
}

/* Password DB - SQLite's shell tries getpwuid() when looking up $HOME. */
struct passwd {
	char *pw_name;
	char *pw_passwd;
	uid_t pw_uid;
	gid_t pw_gid;
	char *pw_gecos;
	char *pw_dir;
	char *pw_shell;
};

/* posix-user (via LIBCRYPTO) provides getpwuid/getpwnam, so gate our stubs out
 * when it's enabled to avoid a multiple-definition error. */
#if !defined(CONFIG_LIBPOSIX_USER)
struct passwd *getpwuid(uid_t uid __attribute__((unused)))
{
	return NULL;
}

struct passwd *getpwnam(const char *name __attribute__((unused)))
{
	return NULL;
}
#endif /* !CONFIG_LIBPOSIX_USER */

/* gen/glob.c support: no environ/login/locale, so secure_getenv->getenv,
 * getlogin->NULL, strcoll->strcmp (C-locale byte compare). */
char *secure_getenv(const char *name)
{
	return getenv(name);
}

/* getlogin is also provided by posix-user - gate out when enabled. */
#if !defined(CONFIG_LIBPOSIX_USER)
char *getlogin(void)
{
	return NULL;
}
#endif /* !CONFIG_LIBPOSIX_USER */

int strcoll(const char *a, const char *b)
{
	return strcmp(a, b);
}

/* time-conversion entries that nolibc's musl-imported subset doesn't ship. */
struct tm;
extern struct tm *__secs_to_tm(long long t, struct tm *tm);

struct tm *localtime_r(const long *timep, struct tm *result)
{
	if (!timep || !result)
		return NULL;
	return __secs_to_tm((long long)*timep, result);
}

struct tm *gmtime_r(const long *timep, struct tm *result)
{
	return localtime_r(timep, result);
}

/* Math stubs SQLite may reference; replace with a real libm if needed. */
double fmod(double x, double y)
{
	if (y == 0.0)
		return 0.0;
	long long q = (long long)(x / y);
	return x - (double)q * y;
}

double floor(double x)
{
	long long i = (long long)x;
	if (x < 0.0 && (double)i != x)
		i--;
	return (double)i;
}

double ceil(double x)
{
	long long i = (long long)x;
	if (x > 0.0 && (double)i != x)
		i++;
	return (double)i;
}

double pow(double x, double y)
{
	double result = 1.0;
	long long iy = (long long)y;
	if ((double)iy != y)
		return 0.0;
	if (iy < 0) {
		x = 1.0 / x;
		iy = -iy;
	}
	while (iy > 0) {
		if (iy & 1)
			result *= x;
		x *= x;
		iy >>= 1;
	}
	return result;
}

double log(double x __attribute__((unused)))
{
	return 0.0;
}

double log10(double x __attribute__((unused)))
{
	return 0.0;
}

double exp(double x __attribute__((unused)))
{
	return 1.0;
}

double sqrt(double x)
{
	if (x < 0.0)
		return 0.0;
	if (x == 0.0)
		return 0.0;
	double guess = x;
	int i;
	for (i = 0; i < 32; i++)
		guess = 0.5 * (guess + x / guess);
	return guess;
}

/* Locale-free ASCII strcasecmp/strncasecmp (FreeBSD's need xlocale). */
static inline int ascii_tolower(int c)
{
	return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
}

int strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *a = (const unsigned char *)s1;
	const unsigned char *b = (const unsigned char *)s2;
	int d;
	for (;;) {
		d = ascii_tolower(*a) - ascii_tolower(*b);
		if (d != 0 || *a == '\0')
			return d;
		a++;
		b++;
	}
}

/* strerror - minimal mapping. FreeBSD's strerror.c needs libssp. */
char *strerror(int errnum)
{
	static char unknown[] = "Unknown error";
	switch (errnum) {
	case 0:  return (char *)"Success";
	case 1:  return (char *)"Operation not permitted";
	case 2:  return (char *)"No such file or directory";
	case 5:  return (char *)"Input/output error";
	case 9:  return (char *)"Bad file descriptor";
	case 11: return (char *)"Resource temporarily unavailable";
	case 12: return (char *)"Cannot allocate memory";
	case 13: return (char *)"Permission denied";
	case 17: return (char *)"File exists";
	case 22: return (char *)"Invalid argument";
	case 28: return (char *)"No space left on device";
	case 38: return (char *)"Function not implemented";
	default: return unknown;
	}
}

/* GNU-style strerror_r - matches nolibc's <string.h> prototype. */
char *strerror_r(int errnum, char *buf, size_t buflen)
{
	const char *s = strerror(errnum);
	if (!buf || buflen == 0)
		return (char *)s;
	size_t i = 0;
	while (s[i] && i + 1 < buflen) { buf[i] = s[i]; i++; }
	buf[i] = '\0';
	return buf;
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	const unsigned char *a = (const unsigned char *)s1;
	const unsigned char *b = (const unsigned char *)s2;
	int d;
	while (n--) {
		d = ascii_tolower(*a) - ascii_tolower(*b);
		if (d != 0 || *a == '\0')
			return d;
		a++;
		b++;
	}
	return 0;
}

double sin(double x __attribute__((unused)))     { return 0.0; }
double cos(double x __attribute__((unused)))     { return 0.0; }
double tan(double x __attribute__((unused)))     { return 0.0; }
double asin(double x __attribute__((unused)))    { return 0.0; }
double acos(double x __attribute__((unused)))    { return 0.0; }
double atan(double x __attribute__((unused)))    { return 0.0; }
double atan2(double y __attribute__((unused)), double x __attribute__((unused))) { return 0.0; }
double sinh(double x __attribute__((unused)))    { return 0.0; }
double cosh(double x __attribute__((unused)))    { return 0.0; }
double tanh(double x __attribute__((unused)))    { return 0.0; }

/* access/getcwd/ftruncate/truncate/link/symlink/readlink/rmdir/pipe/dup/dup2/
 * chown/lchown/fchown are all provided by Unikraft (vfscore/posix-*). */

/* getuid/geteuid/getgid/getegid come from posix-user - gate out when enabled;
 * getpid/getppid are not, so they stay active. */
#if !defined(CONFIG_LIBPOSIX_USER)
uid_t getuid(void)  { return 0; }
uid_t geteuid(void) { return 0; }
gid_t getgid(void)  { return 0; }
gid_t getegid(void) { return 0; }
#endif /* !CONFIG_LIBPOSIX_USER */
pid_t getpid(void)  { return 1; }
pid_t getppid(void) { return 0; }

int isatty(int fd)
{
	return (fd == 0 || fd == 1 || fd == 2) ? 1 : 0;
}

unsigned int alarm(unsigned int seconds __attribute__((unused)))
{
	return 0;
}

/* uname() is provided by libposix_sysinfo. */

int pause(void)
{
	errno = 38;
	return -1;
}

double fabs(double x) { return x < 0.0 ? -x : x; }

double modf(double x, double *iptr)
{
	double t = (x < 0.0) ? -floor(-x) : floor(x);
	if (iptr)
		*iptr = t;
	return x - t;
}

double ldexp(double x, int exp)
{
	double r = x;
	if (exp >= 0) {
		while (exp-- > 0)
			r *= 2.0;
	} else {
		while (exp++ < 0)
			r /= 2.0;
	}
	return r;
}

double frexp(double x, int *exp)
{
	int e = 0;
	if (x != 0.0) {
		double y = x < 0.0 ? -x : x;
		while (y >= 1.0) { y /= 2.0; e++; }
		while (y < 0.5)  { y *= 2.0; e--; }
	}
	if (exp) *exp = e;
	return ldexp(x, -e);
}

double trunc(double x)
{
	long long i = (long long)x;
	return (double)i;
}

double round(double x)
{
	if (x >= 0.0)
		return floor(x + 0.5);
	return ceil(x - 0.5);
}

/* Program-name backing store. getprogname/setprogname are the real FreeBSD
 * sources now; a unikernel has no argv[0], so default it (setprogname may
 * override at runtime). */
const char *__progname = "unikraft";

/* FreeBSD 15's err.c calls uexterr_gettext() for extended error text; this port
 * has none, so report "none" and err()/warn() fall back to strerror(). */
int uexterr_gettext(char *buf, size_t bufsz)
{
	if (buf != NULL && bufsz > 0)
		buf[0] = '\0';
	return -1;	/* no extended error available */
}

/* strftime glue. No tz database, so present a fixed UTC: tzset() is a no-op,
 * tzname[]="UTC", timezone=0 (only %Z/%z notice). These are declared-only in
 * musl's <time.h>, so defining them here is not a multi-def. */
char *tzname[2] = { (char *)"UTC", (char *)"UTC" };
long timezone = 0;
void tzset(void) { }

/* strftime's %s uses timeoff() == timegm(tm) - offset; provide a self-contained
 * UTC timegm (Hinnant's days-from-civil). Only %s/epoch reaches this. */
time_t tz_private_timeoff(struct tm *tm, long offset)
{
	int y = tm->tm_year + 1900;
	int m = tm->tm_mon + 1;			/* 1..12 */
	int d = tm->tm_mday;
	y -= (m <= 2);
	int era = (y >= 0 ? y : y - 399) / 400;
	unsigned yoe = (unsigned)(y - era * 400);
	unsigned doy = (153u * (unsigned)(m + (m > 2 ? -3 : 9)) + 2) / 5 + (unsigned)d - 1;
	unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
	long days = (long)era * 146097 + (long)doe - 719468;	/* days since 1970-01-01 */
	return (time_t)(days * 86400L + tm->tm_hour * 3600L
			+ tm->tm_min * 60L + tm->tm_sec) - offset;
}

/* C-locale time-name table for strftime. timelocal.c (the xlocale runtime) is
 * not built; strftime only needs the function below, so hand back a static
 * C-locale table. struct lc_time_T comes from <timelocal.h>. */
#include "timelocal.h"

static const struct lc_time_T _C_time_locale = {
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" },
	{ "January", "February", "March", "April", "May", "June",
	  "July", "August", "September", "October", "November", "December" },
	{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" },
	{ "Sunday", "Monday", "Tuesday", "Wednesday",
	  "Thursday", "Friday", "Saturday" },
	"%H:%M:%S",			/* X_fmt */
	"%m/%d/%y",			/* x_fmt */
	"%a %b %e %H:%M:%S %Y",		/* c_fmt */
	"AM", "PM",
	"%a %b %e %H:%M:%S %Z %Y",	/* date_fmt */
	{ "January", "February", "March", "April", "May", "June",
	  "July", "August", "September", "October", "November", "December" },
	"md",				/* md_order */
	"%I:%M:%S %p"			/* ampm_fmt */
};

struct lc_time_T *__get_current_time_locale(locale_t loc)
{
	(void)loc;
	return (struct lc_time_T *)&_C_time_locale;
}

/* Locale-aware printf variants used by strftime. On the FreeBSD stdio path the
 * real sources provide them, so define these forwarders only on the nolibc
 * path. C locale only, so the locale argument is ignored. */
#if !defined(CONFIG_LIBFREEBSD_LIBC_STDIO)
int sprintf_l(char *s, void *loc, const char *fmt, ...)
{
	va_list ap;
	int r;
	(void)loc;
	va_start(ap, fmt);
	r = vsprintf(s, fmt, ap);
	va_end(ap);
	return r;
}

int fprintf_l(FILE *fp, void *loc, const char *fmt, ...)
{
	va_list ap;
	int r;
	(void)loc;
	va_start(ap, fmt);
	r = vfprintf(fp, fmt, ap);
	va_end(ap);
	return r;
}
#endif /* !CONFIG_LIBFREEBSD_LIBC_STDIO */
