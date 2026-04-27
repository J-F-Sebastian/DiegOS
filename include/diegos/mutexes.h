/**
 *
 *                                                   ,----..
 *        ,---,                                     /   /   \   .--.--.
 *      .'  .' `\    ,--,                          /   .     : /  /    '.
 *    ,---.'     \ ,--.'|                         .   /   ;.  \  :  /`. /
 *    |   |  .`\  ||  |,                ,----._,..   ;   /  ` ;  |  |--`
 *    :   : |  '  |`--'_       ,---.   /   /  ' /;   |  ; \ ; |  :  ;_
 *    |   ' '  ;  :,' ,'|     /     \ |   :     ||   :  | ; | '\  \    `.
 *    '   | ;  .  |'  | |    /    /  ||   | .\  ..   |  ' ' ' : `----.   \
 *    |   | :  |  '|  | :   .    ' / |.   ; ';  |'   ;  \; /  | __ \  \  |
 *    '   : | /  ; '  : |__ '   ;   /|'   .   . | \   \  ',  / /  /`--'  /
 *    |   | '` ,/  |  | '.'|'   |  / | `---`-'| |  ;   :    / '--'.     /
 *    ;   :  .'    ;  :    ;|   :    | .'__/\_: |   \   \ .'    `--'---'
 *    |   ,.'      |  ,   /  \   \  /  |   :    :    `---`
 *    '---'         ---`-'    `----'    \   \  /
 *                                       `--`-'
 */

/*
 * DiegOS Operating System source code
 *
 * Copyright (C) 2012 - 2026 Diego Gallizioli
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
 * Create a new mutex with the specified name.
 *
 * PARAMETERS IN
 * const char *name - the name of the mutex, for debugging purposes.
 *
 * RETURNS
 * a pointer to the created mutex, or NULL on failure.
 */
mutex_t *thread_create_mutex(const char *name);

/*
 * Locks a mutex.
 *
 * PARAMETERS IN
 * mutex_t *mtx - the mutex to be locked.
 *
 * RETURNS
 * EOK on success
 * EINVAL if the mutex is invalid
 * EBUSY if the thread has already locked the mutex
 */
int thread_lock_mutex(mutex_t * mtx);

/*
 * Locks a mutex.
 * If the mutex is already locked, the thread will be blocked until
 * the mutex is unlocked or the timeout expires.
 *
 * PARAMETERS IN
 * mutex_t *mtx - the mutex to be locked.
 * unsigned msecs - the timeout in milliseconds, if 0, the thread will wait indefinitely.
 *
 * RETURNS
 * EOK on success
 * EINVAL if the mutex is invalid
 * EBUSY if the thread has already locked the mutex
 * ETIMEDOUT if the timeout expires before the mutex is locked
 */
int thread_lock_mutex_timed(mutex_t * mtx, unsigned msecs);

/*
 * Unlocks a mutex.
 *
 * PARAMETERS IN
 * mutex_t *mtx - the mutex to be unlocked.
 *
 * RETURNS
 * EOK on success
 * EINVAL if the mutex is invalid
 * EPERM if the thread does not own the mutex
 */
int thread_unlock_mutex(mutex_t * mtx);

/*
 * Checks if a mutex is locked.
 *
 * PARAMETERS IN
 * mutex_t *mtx - the mutex to be checked.
 *
 * RETURNS
 * TRUE if the mutex is locked, FALSE if it is unlocked or invalid.
 */
BOOL thread_mutex_is_locked(mutex_t * mtx);

/*
 * Destroys a mutex.
 *
 * PARAMETERS IN
 * mutex_t *mtx - the mutex to be destroyed.
 *
 * RETURNS
 * EOK on success
 * EINVAL if the mutex is invalid
 * EBUSY if the mutex is currently locked by any thread
 */
int thread_destroy_mutex(mutex_t * mtx);

#endif				// MUTEXES_H_INCLUDED
