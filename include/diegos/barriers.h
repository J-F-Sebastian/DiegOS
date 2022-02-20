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

#ifndef BARRIERS_H_INCLUDED
#define BARRIERS_H_INCLUDED

#include <diegos/events.h>

/*
 * Barriers API.
 * This API can be used in interrupt context.
 * Barriers are a form of synchronization among threads.
 * A barrier is a shared boolean where TRUE means open, FALSE means closed.
 * When a running thread encounters a closed barrier, the barrier will set the
 * thread to wait state.
 * When a running thread encounters an open barrier, the barrier won't change
 * the thread's state.
 * All threads waiting on a barrier will move to ready state once the barrier
 * is opened.
 * This data structure allow threads to line up and wait for a common event
 * or condition.
 */

typedef struct barrier barrier_t;

/*
 * Create a barrier providing a name (optional).
 *
 * PARAMETERS IN
 * const char *name - barrier name, can be NULL
 * BOOL autoclose   - if a barrier is open and there are pending threads on it,
 *                    the barrier will be automatically closed after resuming
 *                    the threads
 *
 * RETURNS
 * A pointer to a barrier handle or NULL in case of failure.
 */
barrier_t *barrier_create(const char *name, BOOL autoclose);

/*
 * Destroy a barrier. The barrier will be set to open before destruction,
 * hence moving all waiting threads to READY state.
 *
 * PARAMETERS IN
 * barrier_t *barrier - the barrier to be destroyed
 *
 * RETURNS
 * EOK in case of success
 * EINVAL if barrier is invalid
 */
int barrier_done(barrier_t * barrier);

/*
 * Sets a barrier state to open.
 *
 * PARAMETERS IN
 * barrier_t *barrier - barrier handle
 *
 * RETURNS
 * EOK in case of success
 * EINVAL if barrier is invalid
 */
int barrier_open(barrier_t * barrier);

/*
 * Sets a barrier state to open.
 *
 * PARAMETERS IN
 * barrier_t *barrier - barrier handle
 *
 * RETURNS
 * EOK in case of success
 * EINVAL if barrier is invalid
 */
int barrier_close(barrier_t * barrier);

/*
 * Set the running thread in wait state; opening the barrier will
 * resume the thread.
 *
 * PARAMETERS IN
 * barrier_t *barrier - barrier handle
 *
 * RETURNS
 * EOK in case of success
 * EINVAL if barrier is invalid
 * EPERM if the thread cannot be set to wait state
 */
int wait_for_barrier(barrier_t * barrier);

/*
 * Print to stderr the specified barrier's configuration and status.
 * If barrier is null, all barriers are processed.
 *
 * PARAMETERS IN
 * barrier_t *barrier - a specific barrier to be dumped, or NULL to dump'em all
 */
void barrier_dump(const barrier_t * barrier);

#endif
