/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2015 Diego Gallizioli
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

#include	<ctype.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	"loc_time.h"

#define	RULE_LEN	120
#define	TZ_LEN		10

long    timezone = 0;
long    dst_off = 60 * 60;
int	    daylight = 0;

const int days_per_month[2][12] = {
    /* non-leap year */
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    /* leap year */
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};
