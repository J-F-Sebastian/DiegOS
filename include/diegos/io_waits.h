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

#ifndef IO_WAITS_H_INCLUDED
#define IO_WAITS_H_INCLUDED

#include <stdint.h>
#include <libs/list.h>

typedef list_inst wait_queue_t;

/*
 * Initialize a wait queue object.
 * A wait queue is a suspension point for threads
 * which execution can be resumed from interrupt context
 * or another thread context.
 * Using wait queues in drivers is safe as they are interrupt safe.
 * The wait queue object is not managed by this API.
 * No memory management is performed on the object pointed by wq.
 *
 * PARAMETERS IN
 * wait_queue_t *wq - a pointer to a wait_queue_t data structure
 *
 * RETURNS
 * EINVAL if wq is NULL
 * EPERM if wq cannot be added to internal structures
 * EOK in case of success
*/
int thread_io_wait_init(wait_queue_t * wq);

/*
 * Suspend execution and wait to be resumed.
 * Wait duration is undefined.
 *
 * PARAMETERS IN
 * wait_queue_t *wq - a pointer to a wait_queue_t data structure
 *
 * RETURNS
 * EINVAL if wq is NULL
 * EPERM if wq cannot be added to internal structures or the
 *       kernel could not make the caller thread wait for I/O
 * EOK in case of success
 */
int thread_io_wait(wait_queue_t * wq);

/*
 * Suspend execution and wait to be resumed.
 * Wait duration is configured by msecs, if msecs is 0 the behavior
 * is equal to thread_io_wait().
 * If the wait takes longer than msecs milliseconds the caller thread
 * is woken up and ETIMEDOUT is returned.
 *
 * PARAMETERS IN
 * wait_queue_t *wq - a pointer to a wait_queue_t data structure
 *
 * RETURNS
 * EINVAL if wq is NULL
 * EPERM if wq cannot be added to internal structures or the
 *       kernel could not make the caller thread wait for I/O
 * ETIMEDOUT if the wait expired and no wakeup happened
 * EOK in case of success
 */
int thread_io_wait_timed(wait_queue_t * wq, unsigned msecs);

/*
 * Resume all threads waiting on a wait queue.
 * This function is expected to be called from an interrupt context.
 *
* RETURNS
 * EINVAL if wq is NULL
 * EOK in case of success
 */
int thread_io_resume(wait_queue_t * wq);

/*
 * Wake up all waiting threads and release the wait queue, actually
 * destroying the object and freeing memory.
 *
 * PARAMETERS IN
 * wait_queue_t *wq - pointer to a wait queue
 *
 * RETURNS
 * EINVAL if wq is NULL
 * EFAULT if wq is not a known wait queue
 * EOK in case of success
 * */
int thread_io_wait_done(wait_queue_t * wq);

#endif				// IO_WAITS_H_INCLUDED
