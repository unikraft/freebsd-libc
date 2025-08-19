/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Authors: Lord Daniell
 *
 * Copyright (c) 2025, Unikraft GmbH and The Unikraft Authors.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * FreeBSD libc to Unikraft Syscall Redirection Layer
 *
 * This file serves as a compatibility layer to redirect FreeBSD libc's
 * internal function calls to Unikraft's system call layer. It provides
 * minimal, self-contained implementations and stubs for various functions
 * to satisfy the linker and allow FreeBSD libc to be used in a Unikraft
 * environment. This approach avoids the need to override standard library
 * functions directly, instead providing shims that translate FreeBSD's
 * expectations to Unikraft's APIs.
 *
 * The key responsibilities of this file include:
 *  - Redirecting I/O calls (e.g., __sys_write) to Unikraft's VFS.
 *  - Providing a minimal but functional FILE structure for stdio.
 *  - Stubbing out locale, memory allocation, and other complex subsystems
 *    that are not required for basic applications like hello-world.
 *  - Defining a compatible `errno` location.
 *  - Providing a stack-smashing protector (`__chk_fail`) implementation.
 */

/*******************************************************************************
 * SECTION: Type and Constant Definitions
 *
 * This section provides a minimal set of type and constant definitions to
 * make this file self-contained and avoid pulling in a large number of
 * FreeBSD headers. These definitions are compatible with both FreeBSD's
 * expectations and Unikraft's environment.
 ******************************************************************************/

/* Completely self-contained definitions */
typedef unsigned long size_t;
typedef long ssize_t;
typedef int pid_t;
typedef unsigned int mode_t;
typedef long off_t;

/* FreeBSD __sFILE structure for stdio redirection */
struct __sbuf {
    unsigned char *_base;
    int _size;
};

typedef struct __sFILE {
    unsigned char *_p;      /* current position in buffer */
    int _r;                 /* read space left for getc() */
    int _w;                 /* write space left for putc() */
    short _flags;           /* flags */
    short _file;            /* fileno, if Unix descriptor, else -1 */
    struct __sbuf _bf;      /* the buffer */
    int _lbfsize;           /* 0 or -_bf._size, for inline putc */
    void *_cookie;          /* cookie passed to io functions */
    int (*_close)(void *);
    int (*_read)(void *, char *, int);
    long (*_seek)(void *, long, int);  /* Simplified fpos_t as long */
    int (*_write)(void *, const char *, int);
    struct __sbuf _ub;      /* ungetc buffer */
    unsigned char *_up;     /* saved _p when _p is doing ungetc data */
    int _ur;                /* saved _r when _r is counting ungetc data */
    unsigned char _ubuf[3]; /* guarantee an ungetc() buffer */
    unsigned char _nbuf[1]; /* guarantee a getc() buffer */
    struct __sbuf _lb;      /* buffer for fgetln() */
    int _blksize;           /* stat.st_blksize */
    long _offset;           /* current lseek offset - simplified fpos_t */
    void *_fl_mutex;        /* mutex for MT-safety - simplified */
    void *_fl_owner;        /* current owner - simplified */
    int _fl_count;          /* recursive lock count */
    int _orientation;       /* orientation for fwide() */
    char _mbstate[128];     /* multibyte conversion state - simplified */
    int _flags2;            /* additional flags */
} FILE;

/* Standard definitions */
#ifndef NULL
#define NULL ((void*)0)
#endif

#ifndef EOF
#define EOF (-1)
#endif

/* FreeBSD stdio flags */
#define __SRD   0x0004    /* this file is open for reading */
#define __SWR   0x0008    /* this file is open for writing */
#define __SEOF  0x0020    /* found EOF */
#define __SERR  0x0040    /* found error */

/* Additional errno constants */
#ifndef ENOMEM
#define ENOMEM 12
#endif

#ifndef EBADF
#define EBADF 9
#endif

#ifndef EINVAL
#define EINVAL 22
#endif

/* File access modes (basic definitions) */
#ifndef O_RDONLY
#define O_RDONLY 0
#endif

#ifndef O_WRONLY
#define O_WRONLY 1
#endif

#ifndef O_CREAT
#define O_CREAT 64
#endif

#ifndef O_TRUNC
#define O_TRUNC 512
#endif

#ifndef O_APPEND
#define O_APPEND 1024
#endif

/*******************************************************************************
 * SECTION: Stack Smashing Protector
 *
 * Provides a minimal implementation of the stack smashing protector function,
 * `__chk_fail`. This function is called when a stack canary mismatch is
 * detected, indicating a potential buffer overflow.
 ******************************************************************************/
void __chk_fail(void)
{
    /* Simple implementation - trigger crash */
    *((volatile int*)0) = 1; /* Intentional crash */
    __builtin_unreachable();
}

/*******************************************************************************
 * SECTION: Unikraft Syscall Forward Declarations
 *
 * Declares the Unikraft syscall functions that are used in this file. This
 * avoids the need to include the full Unikraft syscall header.
 ******************************************************************************/
extern long uk_syscall_r_write(int fd, const void *buf, size_t count);
extern long uk_syscall_r_read(int fd, void *buf, size_t count);
extern long uk_syscall_r_open(const char *pathname, int flags, mode_t mode);
extern long uk_syscall_r_close(int fd);
extern long uk_syscall_r_lseek(int fd, off_t offset, int whence);
extern int _uk_vprintk(int level, const char *libname, const char *srcname, unsigned int srcline, const char *fmt, __builtin_va_list ap);

/* Simple strlen implementation to avoid dependencies */
static inline size_t strlen(const char *s)
{
    size_t len = 0;
    if (!s) return 0;
    while (s[len]) len++;
    return len;
}

/*******************************************************************************
 * SECTION: Errno Redirection
 *
 * Redirects FreeBSD's `__error()` function to Unikraft's `errno`. This ensures
 * that error codes set by FreeBSD libc are visible to the application.
 ******************************************************************************/
int errno = 0;

int *__error(void)
{
    /* Redirect to Unikraft's errno location */
    return &errno;
}

/*******************************************************************************
 * SECTION: Locale Stubs
 *
 * Provides minimal stubs for locale-related data structures and functions.
 * These are required to satisfy the linker for functions like `printf`, but
 * do not provide any actual locale support.
 ******************************************************************************/
struct __locale_t {
    int dummy;
};

static struct __locale_t global_locale = { 0 };

struct __locale_t *__xlocale_global_locale = &global_locale;
struct __locale_t *__xlocale_C_locale = &global_locale;  /* C locale same as global */
int __has_thread_locale = 0;
/* Use __thread for TLS consistency with FreeBSD expectations */
__thread struct __locale_t *__thread_locale = NULL;

/*******************************************************************************
 * SECTION: Syscall Redirection and Stubs
 *
 * This section contains the core redirection logic, mapping FreeBSD's
 * internal syscall functions (e.g., `__sys_write`) and standard library
 * functions (e.g., `write`) to their Unikraft equivalents. It also provides
 * stubs for other functions that are required for linking but not for basic
 * functionality.
 ******************************************************************************/

/* FreeBSD printf internally calls this for stdout/stderr */
long __sys_write(int fd, const void *buf, size_t count)
{
    /* Redirect to Unikraft syscall */
    return uk_syscall_r_write(fd, buf, count);
}

/* Alternative write interface that FreeBSD might use */
ssize_t write(int fd, const void *buf, size_t count)
{
    long result = uk_syscall_r_write(fd, buf, count);
    return (ssize_t)result;
}

/* Additional system calls needed for linking */
extern long uk_syscall_r_fstat(int fd, void *statbuf);
extern long uk_syscall_r_utimes(const char *pathname, const void *times);

int fstat(int fd, void *statbuf)
{
    long result = uk_syscall_r_fstat(fd, statbuf);
    return (int)result;
}

int utimes(const char *pathname, const void *times)
{
    long result = uk_syscall_r_utimes(pathname, times);
    return (int)result;
}

/* Additional system call stubs */
extern long uk_syscall_r_close(int fd);
extern long uk_syscall_r_futimesat(int dirfd, const char *pathname, const void *times);

int close(int fd)
{
    long result = uk_syscall_r_close(fd);
    return (int)result;
}

int futimesat(int dirfd, const char *pathname, const void *times)
{
    long result = uk_syscall_r_futimesat(dirfd, pathname, times);
    return (int)result;
}

/* qsort stub - not functional but allows linking */
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
    (void)base;
    (void)nmemb;
    (void)size;
    (void)compar;
    /* Simple stub - not functional */
}

/* String function stubs */
char *strdup(const char *s)
{
    (void)s;
    return NULL; /* Simple stub - not functional */
}

int asprintf(char **strp, const char *fmt, ...)
{
    (void)strp;
    (void)fmt;
    return -1; /* Simple stub - not functional */
}

char *strndup(const char *s, size_t n)
{
    (void)s;
    (void)n;
    return NULL; /* Simple stub - not functional */
}

/*******************************************************************************
 * SECTION: FILE Structure Implementation
 *
 * Provides a minimal but functional implementation of the FILE structure and
 * related stdio functions. This allows applications to use standard I/O
 * (stdin, stdout, stderr) and basic file operations (fopen, fclose, etc.).
 ******************************************************************************/

/* Standard stream implementations with proper FreeBSD FILE structure */
static struct __sFILE _stdout_file = {
    ._file = 1,      /* stdout file descriptor */
    ._flags = __SWR, /* write mode */
};

static struct __sFILE _stderr_file = {
    ._file = 2,      /* stderr file descriptor */
    ._flags = __SWR, /* write mode */
};

static struct __sFILE _stdin_file = {
    ._file = 0,      /* stdin file descriptor */
    ._flags = __SRD, /* read mode */
};

/* Export standard streams using FreeBSD naming */
FILE *stdout = (FILE*)&_stdout_file;
FILE *stderr = (FILE*)&_stderr_file;
FILE *stdin = (FILE*)&_stdin_file;

/* Alternative FreeBSD naming */
FILE *__stdoutp = (FILE*)&_stdout_file;
FILE *__stderrp = (FILE*)&_stderr_file;
FILE *__stdinp = (FILE*)&_stdin_file;

/* stdio function stub - use our FILE definition */
int vfprintf(FILE *stream, const char *format, __builtin_va_list ap)
{
    /* For stdout/stderr, use Unikraft's printing */
    if (stream == stdout || stream == stderr || stream == (FILE*)1 || stream == (FILE*)2) {
        return _uk_vprintk(6, "freebsd-libc", "syscall", 0, format, ap); /* KLVL_INFO = 6 */
    }

    /* For other streams, not supported yet */
    return -1;
}

/*******************************************************************************
 * SECTION: Multibyte and String Conversion Stubs
 *
 * Provides minimal stubs for multibyte character support and string conversion
 * functions. These are required as dependencies for ctype and printf.
 ******************************************************************************/

/* Simple locale structure stubs */
static int dummy_rune_locale = 0;
__thread void *_ThreadRuneLocale = &dummy_rune_locale;
void *_CurrentRuneLocale = &dummy_rune_locale;

/* Minimal implementation for printf %d support */
long strtol(const char *nptr, char **endptr, int base)
{
    long result = 0;
    int negative = 0;

    /* Avoid unused parameter warning - base ignored in minimal implementation */
    (void)base;

    if (!nptr) return 0;

    /* Skip whitespace */
    while (*nptr == ' ' || *nptr == '\t') nptr++;

    /* Handle sign */
    if (*nptr == '-') {
        negative = 1;
        nptr++;
    } else if (*nptr == '+') {
        nptr++;
    }

    /* Convert decimal digits only (ignore base for simplicity) */
    while (*nptr >= '0' && *nptr <= '9') {
        result = result * 10 + (*nptr - '0');
        nptr++;
    }

    if (endptr) *endptr = (char*)nptr;
    return negative ? -result : result;
}

/* Locale-aware version - ignore locale for unikernel */
long strtol_l(const char *nptr, char **endptr, int base, void *locale)
{
    /* Avoid unused parameter warning - locale ignored */
    (void)locale;
    return strtol(nptr, endptr, base);
}

/*******************************************************************************
 * SECTION: Stdio Function Redirections
 *
 * This section redirects stdio functions like `puts` and `fputs` to use
 * Unikraft's printing mechanisms. This is essential for getting basic
 * console output to work.
 ******************************************************************************/

/* Forward declarations for Unikraft printing functions */
extern int uk_printk(const char *fmt, ...);
extern int uk_printd(const char *fmt, ...);

/* Forward declaration */
int __vfprintf(FILE *stream, const char *format, __builtin_va_list ap);

/* Core vfprintf implementation - redirect to Unikraft print for stdout/stderr */
int __vfprintf(FILE *stream, const char *format, __builtin_va_list ap)
{
    /* For stdout/stderr, use Unikraft's printing */
    if (stream == stdout || stream == stderr || stream == (FILE*)1 || stream == (FILE*)2) {
        return _uk_vprintk(6, "freebsd-libc", "syscall", 0, format, ap); /* KLVL_INFO = 6 */
    }

    /* For other streams, not supported yet */
    return -1;
}

/* Simple puts implementation that works with original hello.c */
int puts(const char *s)
{
    if (!s) return EOF;

    /* Simple implementation: write to file descriptor 1 (stdout) */
    write(1, s, strlen(s));
    write(1, "\n", 1);
    return 1; /* Success - return non-negative value */
}

/* Alternative puts implementation for completeness */
int fputs(const char *s, FILE *stream)
{
    if (!s || !stream) return EOF;

    /* For stdout/stderr, write to appropriate file descriptor */
    if (stream == stdout || stream == stderr || stream == (FILE*)1 || stream == (FILE*)2) {
        int fd = (stream == stderr || stream == (FILE*)2) ? 2 : 1;
        write(fd, s, strlen(s));
        return 1; /* Success */
    }

    /* For other streams, not supported yet */
    return EOF;
}

/*******************************************************************************
 * SECTION: File I/O Implementation
 *
 * Provides a full file I/O implementation using the FreeBSD FILE structure,
 * but with Unikraft syscalls as the backend. This bridges the gap until the
 * full FreeBSD stdio can be integrated.
 ******************************************************************************/

/* File I/O operations using FreeBSD FILE structure */
FILE *fopen(const char *pathname, const char *mode)
{
    int flags = 0;
    int fd;

    if (!pathname || !mode) {
        errno = EINVAL;
        return NULL;
    }

    /* Parse mode string (basic implementation) */
    switch (mode[0]) {
        case 'r':
            flags = 0; /* O_RDONLY */
            break;
        case 'w':
            flags = 1 | 64 | 512; /* O_WRONLY | O_CREAT | O_TRUNC */
            break;
        case 'a':
            flags = 1 | 64 | 1024; /* O_WRONLY | O_CREAT | O_APPEND */
            break;
        default:
            errno = EINVAL;
            return NULL;
    }

    /* Open file using Unikraft syscall */
    long result = uk_syscall_r_open(pathname, flags, 0644);
    if (result < 0) {
        errno = (int)(-result);
        return NULL;
    }

    fd = (int)result;

    /* Allocate FILE structure (simple static allocation for now) */
    static struct __sFILE file_pool[8]; /* Support up to 8 open files */
    static int file_pool_used[8] = {0};

    int i;
    for (i = 0; i < 8; i++) {
        if (!file_pool_used[i]) {
            file_pool_used[i] = 1;

            /* Initialize FILE structure */
            struct __sFILE *fp = &file_pool[i];
            memset(fp, 0, sizeof(*fp));

            fp->_file = (short)fd;
            if (mode[0] == 'r') {
                fp->_flags = __SRD;
            } else {
                fp->_flags = __SWR;
            }

            return (FILE*)fp;
        }
    }

    /* No free FILE structures */
    uk_syscall_r_close(fd);
    errno = ENOMEM;
    return NULL;
}

int fclose(FILE *stream)
{
    if (!stream) {
        errno = EINVAL;
        return EOF;
    }

    struct __sFILE *fp = (struct __sFILE *)stream;

    /* Close file descriptor */
    if (fp->_file >= 0) {
        long result = uk_syscall_r_close(fp->_file);
        if (result < 0) {
            errno = (int)(-result);
            return EOF;
        }
    }

    /* Mark FILE structure as free (simple implementation) */
    fp->_flags = 0;
    fp->_file = -1;

    return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (!ptr || !stream || size == 0 || nmemb == 0) {
        errno = EINVAL;
        return 0;
    }

    struct __sFILE *fp = (struct __sFILE *)stream;

    if (!(fp->_flags & __SRD)) {
        errno = EBADF; /* Not open for reading */
        return 0;
    }

    size_t total_bytes = size * nmemb;
    long result = uk_syscall_r_read(fp->_file, ptr, total_bytes);

    if (result < 0) {
        errno = (int)(-result);
        fp->_flags |= __SERR;
        return 0;
    }

    if (result == 0) {
        fp->_flags |= __SEOF;
    }

    return (size_t)result / size; /* Return number of complete items read */
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    if (!ptr || !stream || size == 0 || nmemb == 0) {
        errno = EINVAL;
        return 0;
    }

    struct __sFILE *fp = (struct __sFILE *)stream;

    if (!(fp->_flags & __SWR)) {
        errno = EBADF; /* Not open for writing */
        return 0;
    }

    size_t total_bytes = size * nmemb;
    long result = uk_syscall_r_write(fp->_file, ptr, total_bytes);

    if (result < 0) {
        errno = (int)(-result);
        fp->_flags |= __SERR;
        return 0;
    }

    return (size_t)result / size; /* Return number of complete items written */
}

int feof(FILE *stream)
{
    if (!stream) return 0;
    struct __sFILE *fp = (struct __sFILE *)stream;
    return (fp->_flags & __SEOF) != 0;
}

int ferror(FILE *stream)
{
    if (!stream) return 0;
    struct __sFILE *fp = (struct __sFILE *)stream;
    return (fp->_flags & __SERR) != 0;
}

void clearerr(FILE *stream)
{
    if (stream) {
        struct __sFILE *fp = (struct __sFILE *)stream;
        fp->_flags &= ~(__SEOF | __SERR);
    }
}

/* String error function */
char *strerror(int errnum)
{
    static char unknown_msg[] = "Unknown error";

    switch (errnum) {
        case 0: return "Success";
        case 1: return "Operation not permitted";
        case 2: return "No such file or directory";
        case 12: return "Out of memory";
        case 22: return "Invalid argument";
        default:
            return unknown_msg;
    }
}


/* Formatted string printing functions */
int vsnprintf(char *str, size_t size, const char *format, __builtin_va_list ap)
{
    /* Simple stub - not fully functional but allows linking */
    (void)str; (void)size; (void)format; (void)ap;
    return 0; /* Return 0 to indicate no characters written */
}

int fprintf(FILE *stream, const char *format, ...)
{
    __builtin_va_list ap;
    __builtin_va_start(ap, format);
    int ret = vfprintf(stream, format, ap);
    __builtin_va_end(ap);
    return ret;
}

/* File buffer flush function */
int fflush(FILE *stream)
{
    /* Simple stub - no buffering in our minimal implementation */
    (void)stream;
    return 0; /* Success */
}

/*******************************************************************************
 * SECTION: Memory Allocation Stubs
 *
 * Provides minimal stubs for memory allocation functions. These are not
 * functional but are required to satisfy the linker for some libraries.
 ******************************************************************************/
int __mb_sb_limit = 1; /* Single-byte limit for C locale */

void *malloc(size_t size)
{
    (void)size;
    return NULL; /* Simple stub - not functional */
}

void free(void *ptr)
{
    (void)ptr;   /* Simple stub - not functional */
}

void *calloc(size_t nmemb, size_t size)
{
    (void)nmemb;
    (void)size;
    return NULL; /* Simple stub - not functional */
}

void *realloc(void *ptr, size_t size)
{
    (void)ptr;
    (void)size;
    return NULL; /* Simple stub - not functional */
}
