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

#include "loc_time.h"

/* Calculate day of week in proleptic Gregorian calendar. Sunday == 0. */
static int wday(int year, int month, int day)
{
	int adjustment, mm, yy;

	adjustment = (14 - month) / 12;
	mm = month + 12 * adjustment - 2;
	yy = year - adjustment;
	return (day + (13 * mm - 1) / 5 + yy + yy / 4 - yy / 100 + yy / 400) % 7;
}

struct tm *localtime(const time_t * timer)
{
	static struct tm retval = { 0 };
	time_t counter;
	int myvar;
	int secy = 0;

	if (!timer || *timer == 0) {
		return (&retval);
	}

	time_t tmp = *timer;
	tmp -= timezone;
	if (daylight) {
		tmp -= dst_off;
	}

	retval.tm_sec = tmp % 60;	/* seconds after the minute [0, 59] */
	retval.tm_min = (tmp % (60 * 60)) / 60;	/* minutes after the hour [0, 59] */
	retval.tm_hour = (tmp % (24 * 60 * 60)) / (60 * 60);	/* hours since midnight [0, 23] */

	/* count seconds per year up to tmp, keep track of passing years */
	counter = 0;
	myvar = 0;
	while (counter < tmp) {
		myvar++;
		secy = YEARSIZE(myvar);
		secy *= 24 * 60 * 60;
		counter += secy;
	}
	retval.tm_year = (myvar) ? (myvar - 1) : myvar;	/* years since 2000 */

	/* compute remaining seconds in tmp */
	counter -= secy;
	tmp -= counter;

	/* compute remaining days */
	retval.tm_yday = tmp / (24 * 60 * 60);	/* days since January 1 [0, 365] */

	counter = 0;
	myvar = 0;
	while (counter < tmp) {
		secy = days_per_month[LEAPYEAR(retval.tm_year)][myvar];
		secy *= 24 * 60 * 60;
		counter += secy;
		myvar++;
	}
	if (counter != tmp)
		myvar--;
	retval.tm_mon = myvar;	/* months since January [0, 11] */

	/* compute remaining seconds in tmp */
	counter -= tmp;
	retval.tm_mday = (counter / (24 * 60 * 60)) + 1;	/* day of the month [1, 31] */

	retval.tm_wday = wday(retval.tm_year, retval.tm_mon, retval.tm_mday);	/* days since Sunday [0, 6] */

	retval.tm_isdst = daylight;	/* Daylight Saving Time flag */

	return (&retval);
}
