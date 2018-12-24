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

#ifndef _PC_H_
#define _PC_H_

/*
 * Max threads
 */
#define DIEGOS_MAX_THREADS (20)

/*
 * Default names to be used by device API
 */
#define DEFAULT_STDIN   "/dev/uart0"
#define DEFAULT_STDOUT  "/dev/uart0"
#define DEFAULT_STDERR  "/dev/uart0"

/*
 / Device name for kernel TTY, i.e. kprintf
 */
#define DEFAULT_DBG_TTY "vgtty0"

/*
 * Default clock resolution in Hz , i.e. how many clock ticks must happen
 * in a second.
 */
#define DEFAULT_CLOCK_RES   (250UL)

#endif
