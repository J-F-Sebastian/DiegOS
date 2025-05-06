/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2025 Diego Gallizioli
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

/* The starting year in struct tm */
#define	YEAR0		(2000)
/* The EPOCH is January 1st 2015 00:00:00 AM */
#define	EPOCH_YR	(2015)
/* The EPOCH in seconds since YEAR0 */
#define	EPOCH_SECS      (473385600L)
#define	SECS_HOUR	(60L * 60L)
#define	SECS_DAY	(24L * SECS_HOUR)
#define	LEAPYEAR(year)	(!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define	YEARSIZE(year)	(LEAPYEAR(year) ? 366 : 365)

extern long int timezone;
extern long int dst_off;
extern int daylight;
extern char *tzname[2];
extern const int days_per_month[2][12];

/*
 * Convert seconds since YEAR0 to a struct tm
 * and returns a pointer to an internal static memory area.
 * epoch is considered correct w.r.t. the timezone, the
 * daylight saving time, or any other seasonal adjustment.
 * The function will convert epoch making
 * no assumptions on timezone, daylight saving time, or any
 * other seasonal adjustment.
 * the field tm_isdst is set to -1.
 *
 * PARAMETERS IN
 * const time_t *epoch - seconds since YEAR0 to be converted
 *
 * RETURNS
 * A pointer to an internal struct tm filled with the converted values.
 */
struct tm *convtime(const time_t *epoch);
