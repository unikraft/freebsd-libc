/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Overlay <stringlist.h> (nolibc ships none; gen/stringlist.c needs it).
 * Mirrors the fork's self-contained header; kept as an overlay so the fork
 * tree stays pristine.
 */
#ifndef _STRINGLIST_H
#define _STRINGLIST_H

#include <sys/types.h>

/*
 * Simple string list
 */
typedef struct _stringlist {
	char	**sl_str;
	size_t	  sl_max;
	size_t	  sl_cur;
} StringList;

__BEGIN_DECLS
StringList *sl_init(void);
int	 sl_add(StringList *, char *);
void	 sl_free(StringList *, int);
char	*sl_find(StringList *, const char *);
__END_DECLS

#endif /* _STRINGLIST_H */
