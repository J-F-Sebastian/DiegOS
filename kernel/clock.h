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

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <types_common.h>

/*
 * This is an over-simplification, should realistically be
 * an API used to get IDs.
 */
enum clock_client_id {
	/* The scheduler */
	CLK_INST_SCHEDULER,
	/* The alarms subsystem */
	CLK_INST_ALARMS,
	/* The timers subsystem */
	CLK_INST_TIMERS,
	CLK_INST_MAX
};

/*
 * kernel clock callback definition.
 * This prototype has to be used with clock_add_cb
 * and clock_del_db.
 *
 * PARAMS
 * msecs - system counter of milliseconds elapsed since boot time.
 *
 * RETURN VALUES
 * none
 */
typedef void (*kernel_clock_cb)(uint64_t msecs);

/*
 * Init clock library, this will configure the system ticks
 * and the clock resolution.
 * Must be called internally by kernel init routine.
 *
 * RETURN VALUES
 * TRUE if initialization is successfu
 * FALSE in any other case
 */
BOOL clock_init(void);

/*
 * Add a callback to the clock library. Every time the clock expires,
 * the registered callbacks will be invoked, having as an
 * argument the milliseconds elapsed since boot time.
 * Ticks can be dynamic, time between two consecutive calls
 * may vary.
 *
 * PARAMETERS IN
 * kernel_clock_cb cb       - a callback
 * clock_client_id instance - the clock client instance
 *
 * RETURN VALUES
 * TRUE if the callback was properly registered
 * FALSE in any other case
 */
BOOL clock_add_cb(kernel_clock_cb cb, enum clock_client_id instance);

/*
 * Remove a callback from the clock library.
 * The callback is removed only if cb matches a registered entry.
 *
 * PARAMS
 * kernel_clock_cb cb       - a callback
 * clock_client_id instance - the clock client instance
 *
 * RETURN VALUES
 * TRUE if the callback was properly removed
 * FALSE in any other case
 */
BOOL clock_del_cb(kernel_clock_cb cb, enum clock_client_id instance);

/*
 * Set the CLK device to work in periodic mode, i.e. the device
 * will fire an interrupt at every expiration, and will trigger
 * automatically a new countdown to expiration.
 *
 * RETURN VALUES
 * TRUE if the CLK device has been set in periodic mode
 * FALSE in any other case
 */
BOOL clock_set_periodic(void);

/*
 * Set the CLK device to work in one-shot mode, i.e. the device
 * will fire an interrupt on expiration, and then will stop counting.
 * The CLK device must be triggered to start over.
 *
 * RETURN VALUES
 * TRUE if the CLK device has been set in periodic mode
 * FALSE in any other case
 */
BOOL clock_set_oneshot(void);

/*
 * Set the CLK device period, a.k.a. the time between two consecutive expirations.
 * This value can be set at runtime.
 *
 * PARAMETERS IN
 * unsigned ms     - the period for a specific client instance
 * clock_client_id - the client instance
 *
 * RETURN VALUES
 * TRUE if the CLK device has been set in periodic mode
 * FALSE in any other case
 */
BOOL clock_set_period(unsigned ms, enum clock_client_id instance);

#endif
