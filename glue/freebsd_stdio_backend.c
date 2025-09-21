/*
 * FreeBSD libc stdio backend implementation for Unikraft
 * 
 * This file implements the FreeBSD stdio backend functions (__sread, __swrite, 
 * __sseek, __sclose) that bridge FreeBSD stdio operations to Unikraft VFS syscalls.
 * 
 * Architecture:
 * FreeBSD stdio functions -> FreeBSD backend functions -> Unikraft syscalls
 */

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

/* Unikraft syscall declarations */
extern long uk_syscall_r_read(int fd, void *buf, size_t count);
extern long uk_syscall_r_write(int fd, const void *buf, size_t count);
extern long uk_syscall_r_lseek(int fd, off_t offset, int whence);
extern long uk_syscall_r_close(int fd);

/* External errno from syscall_redirect.c */
extern int errno;

/* ============================================================================
 * FreeBSD stdio backend functions - Bridge to Unikraft VFS
 * 
 * These functions are called by FreeBSD stdio implementation and need to
 * map to Unikraft syscalls while maintaining FreeBSD semantics.
 * ============================================================================ */

/**
 * __sread - FreeBSD stdio read backend
 * Called by FreeBSD fread() implementation 
 * 
 * @param cookie: FILE pointer (contains file descriptor)
 * @param buf: buffer to read into
 * @param n: number of bytes to read
 * @return: number of bytes read, or -1 on error
 */
int __sread(void *cookie, char *buf, int n)
{
    /* In FreeBSD stdio, cookie is the FILE pointer, 
     * and _file member contains the file descriptor */
    struct __sFILE *fp = (struct __sFILE *)cookie;
    int fd;
    long result;
    
    if (!fp || n < 0) {
        errno = EINVAL;
        return -1;
    }
    
    fd = fp->_file;  /* Extract file descriptor from FILE structure */
    
    /* Call Unikraft read syscall */
    result = uk_syscall_r_read(fd, buf, (size_t)n);
    
    if (result < 0) {
        /* Convert Unikraft error to errno */
        errno = (int)(-result);
        return -1;
    }
    
    return (int)result;
}

/**
 * __swrite - FreeBSD stdio write backend  
 * Called by FreeBSD fwrite() implementation
 * 
 * @param cookie: FILE pointer (contains file descriptor)
 * @param buf: buffer to write from
 * @param n: number of bytes to write
 * @return: number of bytes written, or -1 on error
 */
int __swrite(void *cookie, const char *buf, int n)
{
    struct __sFILE *fp = (struct __sFILE *)cookie;
    int fd;
    long result;
    
    if (!fp || n < 0) {
        errno = EINVAL;
        return -1;
    }
    
    fd = fp->_file;  /* Extract file descriptor from FILE structure */
    
    /* Call Unikraft write syscall */
    result = uk_syscall_r_write(fd, buf, (size_t)n);
    
    if (result < 0) {
        /* Convert Unikraft error to errno */
        errno = (int)(-result);
        return -1;
    }
    
    return (int)result;
}

/**
 * __sseek - FreeBSD stdio seek backend
 * Called by FreeBSD fseek() implementation
 * 
 * @param cookie: FILE pointer (contains file descriptor) 
 * @param offset: seek offset
 * @param whence: seek direction (SEEK_SET, SEEK_CUR, SEEK_END)
 * @return: new file position, or -1 on error
 */
fpos_t __sseek(void *cookie, fpos_t offset, int whence)
{
    struct __sFILE *fp = (struct __sFILE *)cookie;
    int fd;
    long result;
    
    if (!fp) {
        errno = EINVAL;
        return -1;
    }
    
    fd = fp->_file;  /* Extract file descriptor from FILE structure */
    
    /* Call Unikraft lseek syscall */
    result = uk_syscall_r_lseek(fd, (off_t)offset, whence);
    
    if (result < 0) {
        /* Convert Unikraft error to errno */
        errno = (int)(-result);
        return -1;
    }
    
    return (fpos_t)result;
}

/**
 * __sclose - FreeBSD stdio close backend
 * Called by FreeBSD fclose() implementation
 * 
 * @param cookie: FILE pointer (contains file descriptor)
 * @return: 0 on success, -1 on error  
 */
int __sclose(void *cookie)
{
    struct __sFILE *fp = (struct __sFILE *)cookie;
    int fd;
    long result;
    
    if (!fp) {
        errno = EINVAL;
        return -1;
    }
    
    fd = fp->_file;  /* Extract file descriptor from FILE structure */
    
    /* Call Unikraft close syscall */  
    result = uk_syscall_r_close(fd);
    
    if (result < 0) {
        /* Convert Unikraft error to errno */
        errno = (int)(-result);
        return -1;
    }
    
    return 0;
}

/* ============================================================================
 * Additional helper functions for FreeBSD stdio integration
 * ============================================================================ */

/**
 * _sread, _swrite, _sseek - Alternative function names 
 * Some FreeBSD stdio code may call these directly
 */
int _sread(void *cookie, char *buf, int n)
{
    return __sread(cookie, buf, n);
}

int _swrite(void *cookie, const char *buf, int n) 
{
    return __swrite(cookie, buf, n);
}

fpos_t _sseek(void *cookie, fpos_t offset, int whence)
{
    return __sseek(cookie, offset, whence);
}

/**
 * Direct syscall wrappers for FreeBSD libc compatibility
 * These may be called directly by some FreeBSD stdio functions
 */

int _open(const char *path, int flags, mode_t mode)
{
    /* Forward declare Unikraft open syscall */
    extern long uk_syscall_r_open(const char *pathname, int flags, mode_t mode);
    long result;
    
    result = uk_syscall_r_open(path, flags, mode);
    
    if (result < 0) {
        errno = (int)(-result);
        return -1;  
    }
    
    return (int)result;
}

int _close(int fd)
{
    long result = uk_syscall_r_close(fd);
    
    if (result < 0) {
        errno = (int)(-result);
        return -1;
    }
    
    return 0;
}

/* Forward declaration for complete FILE structure from freebsd_compat.h */
struct __sFILE;  /* This will be fully defined in freebsd_compat.h */