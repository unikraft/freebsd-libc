/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay for FreeBSD's <err.h>. The full err(3) family is ported from
 * lib/libc/gen/err.c; this header declares all of it (err/errx/warn/warnx, the
 * v*/*c variants, err_set_file/err_set_exit). err.c's only non-native deps are
 * _getprogname() and uexterr_gettext() (glue; see <exterr.h>).
 */
#ifndef _ERR_H_
#define _ERR_H_

#include <stdarg.h>
#include <sys/cdefs.h>

__BEGIN_DECLS
void	warn(const char *, ...) __printflike(1, 2);
void	warnc(int, const char *, ...) __printflike(2, 3);
void	warnx(const char *, ...) __printflike(1, 2);
void	vwarn(const char *, va_list) __printflike(1, 0);
void	vwarnc(int, const char *, va_list) __printflike(2, 0);
void	vwarnx(const char *, va_list) __printflike(1, 0);
void	err(int, const char *, ...) __printflike(2, 3) __dead2;
void	errc(int, int, const char *, ...) __printflike(3, 4) __dead2;
void	errx(int, const char *, ...) __printflike(2, 3) __dead2;
void	verr(int, const char *, va_list) __printflike(2, 0) __dead2;
void	verrc(int, int, const char *, va_list) __printflike(3, 0) __dead2;
void	verrx(int, const char *, va_list) __printflike(2, 0) __dead2;
void	err_set_exit(void (*)(int));
void	err_set_file(void *);
__END_DECLS

#endif /* _ERR_H_ */
