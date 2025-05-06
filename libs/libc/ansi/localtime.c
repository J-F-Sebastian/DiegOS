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

#include <time.h>
#include <errno.h>

#include "loc_time.h"

/*
 * Offsets for Gauss's algorithm.
 * First row is for common years.
 * Second row for leap years.
 */
static const int month_offset[2][12] = {
	{0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5},
	{0, 3, 4, 0, 2, 5, 0, 3, 6, 1, 4, 6}
};

/* Calculate day of week with Gauss's Algorithm. Sunday == 0. */
static int wday(int year, int month, int day)
{
	int ofs = month_offset[LEAPYEAR(year)][month];
	year -= 1;

	return ((day + ofs + 5 * (year % 4) + 4 * (year % 100) + 6 * (year % 400)) % 7);
}

/*
 * Not needed anymore, see EPOCH_SECS
 * Keep the code for reference.
 */
#if 0
static time_t adjust_to_epoch()
{
	time_t retval = 0;
	int secy, myvar = YEAR0;

	while (myvar < EPOCH_YR) {
		secy = YEARSIZE(myvar);
		secy *= SECS_DAY;
		retval += secy;
		myvar++;
	}

	return (retval);
}
#endif

struct tm *localtime(const time_t * timer)
{
	time_t adj = EPOCH_SECS;
	static struct tm retval = { 0 };
	time_t counter;
	int myvar;
	int secy = 0;

	if (!timer || (*timer < 0)) {
		errno = EINVAL;
		return (NULL);
	}

	tzset();

	time_t tmp = *timer + adj;
	tmp -= timezone;
	if (daylight) {
		tmp -= dst_off;
	}

	retval.tm_sec = tmp % 60;	/* seconds after the minute [0, 59] */
	retval.tm_min = (tmp % (SECS_HOUR)) / 60;	/* minutes after the hour [0, 59] */
	retval.tm_hour = (tmp % (SECS_DAY)) / (60 * 60);	/* hours since midnight [0, 23] */

	/* count seconds per year up to tmp, keep track of passing years */
	counter = 0;
	myvar = 0;
	while (counter < tmp) {
		secy = YEARSIZE(YEAR0 + myvar);
		secy *= SECS_DAY;
		counter += secy;
		myvar++;
	}
	retval.tm_year = (myvar) ? (myvar - 1) : myvar;	/* years since 2000 */

	/* compute remaining seconds in tmp */
	if (counter != tmp) {
		counter -= secy;
	}

	tmp -= counter;
	/* compute remaining days */
	retval.tm_yday = tmp / (SECS_DAY);	/* days since January 1 [0, 365] */

	counter = 0;
	myvar = 0;
	while (counter < tmp) {
		secy = days_per_month[LEAPYEAR(YEAR0 + retval.tm_year)][myvar];
		secy *= SECS_DAY;
		counter += secy;
		if (++myvar == 12)
			myvar = 0;
	}
	if (counter != tmp) {
		myvar--;
		counter -= secy;
	}
	retval.tm_mon = myvar;	/* months since January [0, 11] */

	/* compute remaining seconds in tmp */
	tmp -= counter;
	retval.tm_mday = (tmp / (SECS_DAY)) + 1;	/* day of the month [1, 31] */

	retval.tm_wday = wday(YEAR0 + retval.tm_year, retval.tm_mon, retval.tm_mday);	/* days since Sunday [0, 6] */

	retval.tm_isdst = daylight;	/* Daylight Saving Time flag */

	return (&retval);
}
