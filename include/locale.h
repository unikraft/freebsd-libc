/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Minimal <locale.h> shim providing the fixed "C" locale. gdtoa (built with
 * USE_LOCALE) reads localeconv()->decimal_point; this port has one C locale
 * whose radix is ".", so we return a single static struct lconv with the
 * standard "C" values. Keeping USE_LOCALE (vs patching it out) keeps patches/
 * empty.
 */
#ifndef _LOCALE_H_
#define _LOCALE_H_

#include <limits.h>      /* CHAR_MAX */
#include <xlocale.h>     /* locale_t */

#ifdef __cplusplus
extern "C" {
#endif

struct lconv {
	char *decimal_point;
	char *thousands_sep;
	char *grouping;
	char *int_curr_symbol;
	char *currency_symbol;
	char *mon_decimal_point;
	char *mon_thousands_sep;
	char *mon_grouping;
	char *positive_sign;
	char *negative_sign;
	char int_frac_digits;
	char frac_digits;
	char p_cs_precedes;
	char p_sep_by_space;
	char n_cs_precedes;
	char n_sep_by_space;
	char p_sign_posn;
	char n_sign_posn;
	char int_p_cs_precedes;
	char int_n_cs_precedes;
	char int_p_sep_by_space;
	char int_n_sep_by_space;
	char int_p_sign_posn;
	char int_n_sign_posn;
};

static inline struct lconv *localeconv(void)
{
	static struct lconv _c_locale = {
		(char *)".", (char *)"", (char *)"",
		(char *)"", (char *)"", (char *)"", (char *)"", (char *)"",
		(char *)"", (char *)"",
		CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
		CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX, CHAR_MAX,
		CHAR_MAX, CHAR_MAX,
	};
	return &_c_locale;
}

/* Locale-argument variant: there is only one locale, so ignore it. */
static inline struct lconv *localeconv_l(locale_t _loc)
{
	(void)_loc;
	return localeconv();
}

#ifdef __cplusplus
}
#endif

#endif /* _LOCALE_H_ */
