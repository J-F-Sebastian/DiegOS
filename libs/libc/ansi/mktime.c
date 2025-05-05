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

#include	<time.h>
#include	<limits.h>
#include	<errno.h>

#include	"loc_time.h"

time_t mktime(struct tm *timeptr)
{
	long day;
	int tm_year;
	int yday, month;
	long seconds;
	long int dst;
	struct tm copy;

	if (!timeptr)
		return (time_t) (-1);

	if (timeptr->tm_year < (EPOCH_YR - YEAR0))
		return (time_t) (-1);

	copy = *timeptr;

	copy.tm_min += copy.tm_sec / 60;
	copy.tm_sec %= 60;
	if (copy.tm_sec < 0) {
		copy.tm_sec += 60;
		copy.tm_min--;
	}
	copy.tm_hour += copy.tm_min / 60;
	copy.tm_min = copy.tm_min % 60;
	if (copy.tm_min < 0) {
		copy.tm_min += 60;
		copy.tm_hour--;
	}
	day = copy.tm_hour / 24;
	copy.tm_hour = copy.tm_hour % 24;
	if (copy.tm_hour < 0) {
		copy.tm_hour += 24;
		day--;
	}
	copy.tm_year += copy.tm_mon / 12;
	copy.tm_mon %= 12;
	if (copy.tm_mon < 0) {
		copy.tm_mon += 12;
		copy.tm_year--;
	}
	day += (copy.tm_mday - 1);
	while (day < 0) {
		if (--copy.tm_mon < 0) {
			copy.tm_year--;
			copy.tm_mon = 11;
		}
		day += days_per_month[LEAPYEAR(YEAR0 + copy.tm_year)][copy.tm_mon];
	}
	while (day >= days_per_month[LEAPYEAR(YEAR0 + copy.tm_year)][copy.tm_mon]) {
		day -= days_per_month[LEAPYEAR(YEAR0 + copy.tm_year)][copy.tm_mon];
		if (++(copy.tm_mon) == 12) {
			copy.tm_mon = 0;
			copy.tm_year++;
		}
	}
	copy.tm_mday = day + 1;
	tzset();
	seconds = 0;
	day = 0;		/* means days since day 0 now */

	/*
	 * Assume that when day becomes negative, there will certainly
	 * be overflow on seconds.
	 * The check for overflow needs not to be done for leapyears
	 * divisible by 400.
	 * The code only works when year (2015) is not a leapyear.
	 */
#if LEAPYEAR(EPOCH_YR)
#error	EPOCH_YR is a Leap Year !!!
#endif
	tm_year = copy.tm_year + YEAR0;

	/* OVERFLOW */
	if (LONG_MAX / 365 < tm_year - EPOCH_YR)
		goto overflow;

	day = (tm_year - EPOCH_YR) * 365;
	/* OVERFLOW */
	if (LONG_MAX - day < (tm_year - EPOCH_YR) / 4 + 1)
		goto overflow;

	day += (tm_year - EPOCH_YR) / 4 + ((tm_year % 4) && tm_year % 4 < EPOCH_YR % 4);
	day -= (tm_year - EPOCH_YR) / 100 + ((tm_year % 100) && tm_year % 100 < EPOCH_YR % 100);
	day += (tm_year - EPOCH_YR) / 400 + ((tm_year % 400) && tm_year % 400 < EPOCH_YR % 400);

	yday = month = 0;
	while (month < copy.tm_mon) {
		yday += days_per_month[LEAPYEAR(tm_year)][month];
		month++;
	}
	yday += (copy.tm_mday - 1);
	/* OVERFLOW */
	if (day + yday < 0)
		goto overflow;

	day += yday;

	copy.tm_yday = yday;
	copy.tm_wday = (day + 4) % 7;	/* day 0 was thursday (4) */

	seconds = ((copy.tm_hour * 60L) + copy.tm_min) * 60L + copy.tm_sec;

	/* OVERFLOW */
	if ((LONG_MAX - seconds) / SECS_DAY < day)
		goto overflow;

	seconds += day * SECS_DAY;

	/* Now adjust according to timezone and daylight saving time */

	/* OVERFLOW */
	if (((timezone > 0) && (LONG_MAX - timezone < seconds))
	    || ((timezone < 0) && (seconds < -timezone)))
		goto overflow;

	seconds += timezone;

	if (copy.tm_isdst > 0) {
		dst = dst_off;
	} else if (copy.tm_isdst == 0) {
		dst = 0;
	} else {
		dst = (daylight) ? dst_off : 0;
		copy.tm_isdst = daylight;
	}

	/* dst is always non-negative */
	/* OVERFLOW */
	if (dst > seconds)
		goto overflow;

	seconds -= dst;

	/*
	 * We are fine now. Copy local computed values back to timeptr.
	 */
	*timeptr = copy;
	return (time_t) (seconds);

 overflow:
	errno = EOVERFLOW;
	return (time_t) (-1);
}
