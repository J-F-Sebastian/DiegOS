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
#if 0
#define NULL    ((void *)0)

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif
#endif
#ifndef _TIME_T
#define _TIME_T
typedef long time_t;		/* time in sec since 1 Jan 1970 0000 GMT */
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef long clock_t;		/* time in ticks since process started */
#endif

struct tm {
  int tm_sec;			/* seconds after the minute [0, 59] */
  int tm_min;			/* minutes after the hour [0, 59] */
  int tm_hour;			/* hours since midnight [0, 23] */
  int tm_mday;			/* day of the month [1, 31] */
  int tm_mon;			/* months since January [0, 11] */
  int tm_year;			/* years since 1900 */
  int tm_wday;			/* days since Sunday [0, 6] */
  int tm_yday;			/* days since January 1 [0, 365] */
  int tm_isdst;			/* Daylight Saving Time flag */
};

extern char *tzname[];

_PROTOTYPE( clock_t clock, (void)					);
_PROTOTYPE( double difftime, (time_t _time1, time_t _time0)		);
_PROTOTYPE( time_t mktime, (struct tm *_timeptr)			);
_PROTOTYPE( time_t time, (time_t *_timeptr)				);
_PROTOTYPE( char *asctime, (const struct tm *_timeptr)			);
_PROTOTYPE( char *ctime, (const time_t *_timer)			);
_PROTOTYPE( struct tm *gmtime, (const time_t *_timer)			);
_PROTOTYPE( struct tm *localtime, (const time_t *_timer)		);
_PROTOTYPE( size_t strftime, (char *_s, size_t _max, const char *_fmt,
				const struct tm *_timep)		);

#ifdef _POSIX_SOURCE
_PROTOTYPE( void tzset, (void)						);
#endif

#endif /* _TIME_H */
