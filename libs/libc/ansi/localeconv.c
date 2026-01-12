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

/*
 * localeconv - set components of a struct according to current locale
 */
/* $Header: localeconv.c,v 1.2 89/12/18 15:48:58 eck Exp $ */

#include	<limits.h>
#include	<locale.h>

extern struct lconv _lc;

struct lconv *localeconv(void)
{
	struct lconv *lcp = &_lc;

	lcp->decimal_point = ".";
	lcp->thousands_sep = "";
	lcp->grouping = "";
	lcp->int_curr_symbol = "";
	lcp->currency_symbol = "";
	lcp->mon_decimal_point = "";
	lcp->mon_thousands_sep = "";
	lcp->mon_grouping = "";
	lcp->positive_sign = "";
	lcp->negative_sign = "";
	lcp->int_frac_digits = CHAR_MAX;
	lcp->frac_digits = CHAR_MAX;
	lcp->p_cs_precedes = CHAR_MAX;
	lcp->p_sep_by_space = CHAR_MAX;
	lcp->n_cs_precedes = CHAR_MAX;
	lcp->n_sep_by_space = CHAR_MAX;
	lcp->p_sign_posn = CHAR_MAX;
	lcp->n_sign_posn = CHAR_MAX;

	return lcp;
}
