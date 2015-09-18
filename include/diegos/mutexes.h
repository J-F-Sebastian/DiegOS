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

#ifndef MUTEXES_H_INCLUDED
#define MUTEXES_H_INCLUDED

#include <types_common.h>

typedef struct mutex mutex_t;

/*
 * mutexes management
 */
mutex_t *thread_create_mutex(const char *name);

void thread_lock_mutex(mutex_t *mtx);

void thread_unlock_mutex(mutex_t *mtx);

BOOL thread_mutex_is_locked(mutex_t *mtx);

void thread_destroy_mutex(mutex_t *mtx);

#endif // MUTEXES_H_INCLUDED
