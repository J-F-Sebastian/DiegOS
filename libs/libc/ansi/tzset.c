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
#include <string.h>

#include "loc_time.h"

/*
 * This list is BY FAR incomplete and lacks any kind of serious management
 * of daylight saving times, time zones, etc.
 */
static struct timezones {
	char nt[5], dst[5];
	long int tz, dstofs;
} tzlist[] = {
	{
	"UTC", "BST", 0, +60},
	    /* British Summer Time              UTC +1:00 */
	{
	"CET", "CEST", -60, +60},
	    /* Central European Summer Time UTC +2:00 */
	    /* Central European Time UTC +1:00 */
	{
	"EET", "EEST", -120, +60},
	    /* EEST     Eastern European Summer Time UTC +3:00 */
	    /* EET      Eastern European Time UTC +2:00 */
	{
	"WET", "WEST", 0, +60}
};

char *tzname[2];

void tzset(void)
{
	tzname[0] = tzlist[timezone].nt;
	tzname[1] = tzlist[timezone].dst;
	timezone = tzlist[timezone].tz;
	dst_off = tzlist[timezone].dstofs;
}
