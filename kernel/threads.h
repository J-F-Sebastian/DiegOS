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

#ifndef THREADS_H_INCLUDED
#define THREADS_H_INCLUDED

#include "threads_data.h"

BOOL init_thread_lib(void);

uint8_t init_thread(const char *name,
		    uint8_t prio, void (*entry_ptr) (void), void *stack, uint32_t stack_size);

BOOL done_thread(uint8_t tid);

thread_t *get_thread(uint8_t tid);

const char *state2str(uint8_t state);

void check_thread_stack(void);

#endif				// THREADS_H_INCLUDED
