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

#include <limits.h>
#include <stdint.h>
#include <diegos/delays.h>

#include "kprintf.h"
#include "platform_include.h"

#define BASE_VALUE (1UL << 10)

unsigned long loops_per_msec;

unsigned long loops_per_second()
{
	return (loops_per_msec * 1000);
}

void calibrate_delay(unsigned long (*tickfn) (void))
{
	unsigned long oldticks;
	unsigned long finder;

	finder = BASE_VALUE;

	while (1) {
		oldticks = tickfn();

		/*
		 * wait for the tick to be updated
		 */
		while (oldticks == tickfn()) {
		};
		oldticks = tickfn();
		delay_loop(finder);
		/*
		 * If we wasted enough time to make the tickfn() value
		 * change, then break out.
		 * Else we need to start over and try a larger time to be wasted.
		 */
		if (oldticks < tickfn()) {
			break;
		} else {
			finder += BASE_VALUE;
		}
	}

	/*
	 * We know finder's value is overestimating the correct value
	 */
	while (1) {
		oldticks = tickfn();

		/*
		 * wait for the tick to be updated
		 */
		while (oldticks == tickfn()) {
		};
		oldticks = tickfn();
		delay_loop(finder);
		/*
		 * The goal is to reduce resolution to 0 while
		 * converging the finder value.
		 * If we wait too much (finder too big) we reduce it,
		 * if we wait too few (finder too small) we increase it.
		 * The resolution is halved at every cycle.
		 */
		if (oldticks < tickfn()) {
			finder -= 1;
		} else {
			break;
		}
	}

	loops_per_msec = finder;
}

void mdelay(unsigned long msecs)
{
	delay_loop(loops_per_msec * msecs);
}

void udelay(unsigned long usecs)
{
	unsigned long temp = loops_per_msec * usecs / 1000;
	if (((loops_per_msec * usecs) % 1000U) > 499)
		temp++;

	delay_loop(temp);
}

void ndelay(unsigned long nsecs)
{
	unsigned long temp = loops_per_msec * nsecs / 1000000;
	if (((loops_per_msec * nsecs) % 1000000U) > 499999)
		temp++;

	delay_loop(temp);
}
