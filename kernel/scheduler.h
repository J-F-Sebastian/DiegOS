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

#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include "threads_data.h"

BOOL scheduler_init(void);

void schedule_thread(void);

void update_schedule(void);

BOOL scheduler_suspend_thread(void);

BOOL scheduler_resume_thread(uint8_t flags, uint8_t tid);

BOOL scheduler_delay_thread(uint64_t msecs);

BOOL scheduler_wait_thread(uint8_t flags, uint64_t msecs);

BOOL scheduler_add_thread(uint8_t tid);

BOOL scheduler_remove_thread(uint8_t tid);

uint8_t scheduler_running_tid(void);

thread_t *scheduler_running_thread(void);

uint8_t scheduler_ready_threads(uint8_t prio);

void scheduler_fail_safe(void);

#endif				// SCHEDULER_H_INCLUDED
