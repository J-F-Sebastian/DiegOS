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

#include <stdio.h>
#include <string.h>
#include <time.h>

#include	"loc_time.h"

static const char *days_of_week[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

static const char *months[] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

static const char datefrmt1[] = "%.3s %.3s%3d %.2d:%.2d:%.2d %d";

char *asctime(const struct tm *timeptr)
{
	static char buf[26] = { 0 };

	memset(buf, 0, sizeof(buf));

	if (timeptr) {
		snprintf(buf,
			 sizeof(buf),
			 datefrmt1,
			 days_of_week[timeptr->tm_wday % 7],
			 months[timeptr->tm_mon % 12],
			 timeptr->tm_mday,
			 timeptr->tm_hour,
			 timeptr->tm_min, timeptr->tm_sec, timeptr->tm_year + YEAR0);
	}

	return buf;
}
