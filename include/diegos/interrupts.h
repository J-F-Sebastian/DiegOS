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

#ifndef _INTERRUPTS_H_
#define _INTERRUPTS_H_

/*
 * The following externs *must* be defined for
 * every processor in use. the platform makefile
 * *must* link these symbols. 
 */

#include <types_common.h>

typedef BOOL (*int_handler_t)(void);
typedef void (*exc_handler_t)(void);

/*
 * disable all interrupts
 */
extern void lock(void);

/*
 * enable all interrupts
 */
extern void unlock(void);

/*
 * enable a specific interrupt
 */
extern void enable_int(unsigned intno);

/*
 * disable a specific interrupt
 */
extern void disable_int(unsigned intno);

/*
 * add an interrupt callback
 */
extern int add_int_cb(int_handler_t cb, unsigned intno);

/*
 * remove an interrupt callback
 */
extern int del_int_cb(unsigned intno);

/*
 * add an exception callback
 */
extern int add_exc_cb(exc_handler_t cb, unsigned exc);

/*
 * remove an exception callback
 */
extern int del_exc_cb(unsigned exc);

#endif
