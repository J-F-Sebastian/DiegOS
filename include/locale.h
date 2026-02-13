/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2026 Diego Gallizioli
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* The <locale.h> header is used to custom tailor currency symbols, decimal
 * points, and other items to the local style.  It is ANSI's attempt at
 * avoiding cultural imperialism.  The locale given below is for C.
 */

#ifndef _LOCALE_H
#define _LOCALE_H

#ifndef NULL
#define NULL (void *)0
#endif

struct lconv {
	char *decimal_point;	/* "." */
	char *thousands_sep;	/* ""  */
	char *grouping;		/* ""  */
	char *int_curr_symbol;	/* ""  */
	char *currency_symbol;	/* ""  */
	char *mon_decimal_point;	/* ""  */
	char *mon_thousands_sep;	/* ""  */
	char *mon_grouping;	/* ""  */
	char *positive_sign;	/* ""  */
	char *negative_sign;	/* ""  */
	char int_frac_digits;	/* CHAR_MAX */
	char frac_digits;	/* CHAR_MAX */
	char p_cs_precedes;	/* CHAR_MAX */
	char p_sep_by_space;	/* CHAR_MAX */
	char n_cs_precedes;	/* CHAR_MAX */
	char n_sep_by_space;	/* CHAR_MAX */
	char p_sign_posn;	/* CHAR_MAX */
	char n_sign_posn;	/* CHAR_MAX */
};

#define LC_ALL             1
#define LC_COLLATE         2
#define LC_CTYPE           3
#define LC_MONETARY        4
#define LC_NUMERIC         5
#define LC_TIME            6

/* Function Prototypes. */
char *setlocale(int category, const char *locale);
struct lconv *localeconv(void);

#endif				/* _LOCALE_H */
