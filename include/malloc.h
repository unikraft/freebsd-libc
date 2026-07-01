/*
 * Minimal <malloc.h> overlay for lib-freebsd-libc.
 *
 * <malloc.h> is a Linux/glibc-ism (BSD/POSIX put malloc & friends in <stdlib.h>).
 * nginx's ngx_linux_config.h includes it. We forward to <stdlib.h>, which our
 * overlay already declares malloc/free/calloc/realloc/posix_memalign/memalign.
 */
#ifndef _LIBFREEBSD_MALLOC_H_
#define _LIBFREEBSD_MALLOC_H_

#include <stdlib.h>

#endif /* _LIBFREEBSD_MALLOC_H_ */
