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

#ifndef KERNEL_H_INCLUDED
#define KERNEL_H_INCLUDED

/*******************************************
 *********** HEADER FILE *******************
 *******************************************
 *
 * Name: kernel.h
 * Author: Diego Gallizioli
 * Date: feb 2012
 *******************************************
 * Description: Include file for DiegOS kernel
 * procedures.
 *
 *******************************************/

#include <types_common.h>

/*
 * As explained in threads.h, higher priorities
 * have lower values. The highest priority is
 * always 0. The lowest is up to this enumeration.
  */
typedef enum {
	THREAD_PRIO_REALTIME,
	THREAD_PRIO_HIGH,
	THREAD_PRIO_NORMAL,
	THREAD_PRIO_IDLE
} diegos_prio_t;

/*
 * thread functions for voluntary suspension,
 * delaying or termination.
 */
void thread_suspend(void);

void thread_may_suspend(void);

void thread_delay(unsigned msecs);

void thread_terminate(void);

/*
 * thread management for out-of-context
 * creation or termination.
 */
BOOL thread_create(const char *name,
		   diegos_prio_t prio,
		   void (*entry_ptr)(void), void *stack, unsigned stack_size, uint8_t * tid);

void thread_kill(uint8_t tid);

uint8_t my_thread_id(void);

const char *my_thread_name(void);

#endif				// KERNEL_H_INCLUDED
