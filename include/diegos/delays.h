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

#ifndef _DELAYS_H_
#define _DELAYS_H_

/*
 * Returns the number of loops (smallest, fastest no-op loops) that can be
 * performed in one second.
 *
 * RETURNS
 * unsigned long - number of loops per second
 */
unsigned long loops_per_second(void);

/*
 * This function should be invoked at boot time (much probably in platform
 * dependent code) to calibrate the internal delay loop.
 * The function passed in as a parameter must return a value updating every millisecond.
 * Whenever a millisecond has expired, this function must return a different value.
 * The returned value must be different from one millisecond to the next one.
 * The returned value must be monotonic, i.e. it must increase towards positive values.
 *
 * PARAMETERS IN
 * unsigned long (*tickfn(void)) - function returning an updated value every millisecond.
 */
void calibrate_delay(unsigned long (*tickfn) (void));

/*
 * Delay execution by at least msecs milliseconds.
 * The call is NOT suspensive.
 * CPU will enter a no-op loop and no other activity (except for interrupts)
 * will be served meanwhile.
 */
void mdelay(unsigned long msecs);

/*
 * Delay execution by at least usecs microseconds.
 * The call is NOT suspensive.
 * CPU will enter a no-op loop and no other activity (except for interrupts)
 * will be served meanwhile.
 */
void udelay(unsigned long usecs);

/*
 * Delay execution by at least nsecs nanoseconds.
 * The call is NOT suspensive.
 * CPU will enter a no-op loop and no other activity (except for interrupts)
 * will be served meanwhile.
 */
void ndelay(unsigned long nsecs);

#endif
