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

#ifndef _TIME_H_
#define _TIME_H_

#include <stddef.h>

#define CLOCKS_PER_SEC (DEFAULT_CLOCK_RES)

/* time in sec since 1 Jan 2015 GMT */
typedef long int time_t;

/* time in ticks since process started */
typedef unsigned long long int clock_t;

struct tm {
	int tm_sec;		/* seconds after the minute [0, 59] */
	int tm_min;		/* minutes after the hour [0, 59] */
	int tm_hour;		/* hours since midnight [0, 23] */
	int tm_mday;		/* day of the month [1, 31] */
	int tm_mon;		/* months since January [0, 11] */
	int tm_year;		/* years since 2000 */
	int tm_wday;		/* days since Sunday [0, 6] */
	int tm_yday;		/* days since January 1 [0, 365] */
	int tm_isdst;		/* Daylight Saving Time flag */
};

clock_t clock(void);
double difftime(time_t end, time_t beginning);
time_t mktime(struct tm *timeptr);
time_t time(time_t * timeptr);
char *asctime(const struct tm *timeptr);
char *ctime(const time_t * timer);
struct tm *gmtime(const time_t * timer);
struct tm *localtime(const time_t * timer);
size_t strftime(char *s, size_t max, const char *fmt, const struct tm *timep);
void tzset(void);

#endif
