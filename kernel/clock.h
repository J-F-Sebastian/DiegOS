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
 * kernel clock callback definition.
 * This prototype has to be used with clock_add_cb
 * and clock_del_db.
 *
 * PARAMS
 * ticks - system counter of ticks elapsed since boot time.
 *
 * RETURN VALUES
 * none
 */
typedef void (*kernel_clock_cb)(uint64_t ticks);

/*
 * Init clock library, this will configure the system ticks
 * and the clock resolution.
 * Must be called internally by kernel init routine.
 *
 * RETURN VALUES
 * TRUE if initliazation succeded
 * FALSE in any other case
 */
BOOL clock_init (void);

/*
 * Add a callback to the clock library. Every system tick
 * the registered callbacks will be invoked, having as an
 * argument the actual system tick.
 */
BOOL clock_add_cb (kernel_clock_cb cb);

/*
 * Remove a callback from the clock library.
 */
BOOL clock_del_cb (kernel_clock_cb cb);

/*
 * Get system ticks.
 *
 * RETURN VALUES
 * 64 bit counter of ticks elapsed since boot time.
 */
uint64_t clock_get_ticks (void);

/*
 * Convert milliseconds into system ticks.
 *
 * RETURN VALUES
 * 64 bit counter of the ticks elapsed in msecs.
 */
uint64_t clock_convert_msecs_to_ticks (unsigned msecs);

#endif

