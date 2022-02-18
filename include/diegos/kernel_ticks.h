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

#ifndef _KERNEL_TICKS_H_
#define _KERNEL_TICKS_H_

#include <types_common.h>

/*
 * Get system ticks.
 * A system tick is computed virtually considering
 * DEFAULT_CLOCK_RES as its frequency.
 * One second equals DEFAULT_CLOCK_RES system ticks.
 * The frequency of the CLK device is indipendent from
 * the system ticks frequency.
 *
 * RETURN VALUES
 * 64 bit counter of ticks elapsed since boot time.
 */
uint64_t clock_get_ticks(void);

/*
 * Get system seconds.
 *
 * RETURN VALUES
 * 64 bit counter of seconds elapsed since boot time.
 */
uint64_t clock_get_seconds(void);

/*
 * Get system millliseconds.
 *
 * RETURN VALUES
 * 64 bit counter of milliseconds elapsed since boot time.
 */
uint64_t clock_get_milliseconds(void);

/*
 * Convert milliseconds into system ticks.
 *
 * RETURN VALUES
 * 64 bit counter of ticks elapsed in msecs.
 */
uint64_t clock_convert_msecs_to_ticks(unsigned msecs);

/*
 * Set boot-time seconds, the initial amount of elapsed seconds
 * as if the clock had ticked in the past.
 * This is the value to be set to compute the current date and time.
 * This value must match the date and time of the boot time.
 *
 * RETURN VALUES
 * None
 */
void clock_set_boot_seconds(unsigned seconds);

/*
 * Get boot time seconds.
 *
 * RETURN VALUES
 * unsigned counter of seconds elapsed "before" boot time.
 */
unsigned clock_get_boot_seconds(void);

#endif
