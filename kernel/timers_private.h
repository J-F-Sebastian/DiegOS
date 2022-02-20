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

#ifndef _TIMERS_PRIVATE_H_
#define _TIMERS_PRIVATE_H_

/*
 * Initialize the timers library.
 * Must be called internally by the kernel init
 * routine.
 *
 * RETURN VALUES
 *
 * TRUE if initialization succeded
 * FALSE in any other case
 */
BOOL init_timers_lib(void);

/*
 * The timers handling thread.
 * This thread is in charge of executing all expired callbacks
 * set in the timers data structures.
 * All callbacks are executed in this thread context.
 */
void timers_thread_entry(void);

#endif
