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

#include	"loc_time.h"

time_t mktime (struct tm *timeptr)
{
    long day, year;
	int tm_year;
	int yday, month;
	long seconds;
	int overflow;
	long int dst;

	timeptr->tm_min += timeptr->tm_sec / 60;
	timeptr->tm_sec %= 60;
	if (timeptr->tm_sec < 0) {
		timeptr->tm_sec += 60;
		timeptr->tm_min--;
	}
	timeptr->tm_hour += timeptr->tm_min / 60;
	timeptr->tm_min = timeptr->tm_min % 60;
	if (timeptr->tm_min < 0) {
		timeptr->tm_min += 60;
		timeptr->tm_hour--;
	}
	day = timeptr->tm_hour / 24;
	timeptr->tm_hour= timeptr->tm_hour % 24;
	if (timeptr->tm_hour < 0) {
		timeptr->tm_hour += 24;
		day--;
	}
	timeptr->tm_year += timeptr->tm_mon / 12;
	timeptr->tm_mon %= 12;
	if (timeptr->tm_mon < 0) {
		timeptr->tm_mon += 12;
		timeptr->tm_year--;
	}
	day += (timeptr->tm_mday - 1);
	while (day < 0) {
		if(--timeptr->tm_mon < 0) {
			timeptr->tm_year--;
			timeptr->tm_mon = 11;
		}
		day += days_per_month[LEAPYEAR(YEAR0 + timeptr->tm_year)][timeptr->tm_mon];
	}
	while (day >= days_per_month[LEAPYEAR(YEAR0 + timeptr->tm_year)][timeptr->tm_mon]) {
		day -= days_per_month[LEAPYEAR(YEAR0 + timeptr->tm_year)][timeptr->tm_mon];
		if (++(timeptr->tm_mon) == 12) {
			timeptr->tm_mon = 0;
			timeptr->tm_year++;
		}
	}
	timeptr->tm_mday = day + 1;
	tzset();
    year = EPOCH_YR;
	if (timeptr->tm_year < year - YEAR0) return (time_t)-1;
	seconds = 0;
	day = 0;			/* means days since day 0 now */
	overflow = 0;

	/* Assume that when day becomes negative, there will certainly
	 * be overflow on seconds.
	 * The check for overflow needs not to be done for leapyears
	 * divisible by 400.
	 * The code only works when year (1970) is not a leapyear.
	 */
#if LEAPYEAR(EPOCH_YR)
#error	EPOCH_YR is a Leap Year !!!
#endif
	tm_year = timeptr->tm_year + YEAR0;

	if (LONG_MAX / 365 < tm_year - year) overflow++;
	day = (tm_year - year) * 365;
	if (LONG_MAX - day < (tm_year - year) / 4 + 1) overflow++;
	day += (tm_year - year) / 4
		+ ((tm_year % 4) && tm_year % 4 < year % 4);
	day -= (tm_year - year) / 100
		+ ((tm_year % 100) && tm_year % 100 < year % 100);
	day += (tm_year - year) / 400
		+ ((tm_year % 400) && tm_year % 400 < year % 400);

	yday = month = 0;
	while (month < timeptr->tm_mon) {
		yday += days_per_month[LEAPYEAR(tm_year)][month];
		month++;
	}
	yday += (timeptr->tm_mday - 1);
	if (day + yday < 0) overflow++;
	day += yday;

	timeptr->tm_yday = yday;
	timeptr->tm_wday = (day + 4) % 7;		/* day 0 was thursday (4) */

	seconds = ((timeptr->tm_hour * 60L) + timeptr->tm_min) * 60L + timeptr->tm_sec;

	if ((LONG_MAX - seconds) / SECS_DAY < day) overflow++;
	seconds += day * SECS_DAY;

	/* Now adjust according to timezone and daylight saving time */

	if (((timezone > 0) && (LONG_MAX - timezone < seconds))
	    || ((timezone < 0) && (seconds < - timezone)))
		overflow++;
	seconds += timezone;

	if (timeptr->tm_isdst) {
		dst = dst_off;
    } else {
        dst = 0;
    }

	if (dst > seconds) overflow++;	/* dst is always non-negative */
	seconds -= dst;

	if (overflow) return (time_t)-1;

	return (time_t)seconds;
}
