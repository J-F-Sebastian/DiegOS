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

#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include "threads_data.h"

/*
 * Initializes the scheduler, must be called before any other scheduler procedure.
 * This procedure initializes the ready queues, the delayed queue,
 * the wait queue and the dead queue.
 *
 * RETURNS
 * TRUE on success
 * FALSE on failure.
 */
BOOL scheduler_init(void);

/*
 * Schedules the next thread to be executed.
 * This function is called by the scheduler to select the next running thread
 * from the ready queue.
 * The running thread may not change after this call.
 * This function calls update_schedule() to update the scheduler state
 * and re-schedule threads if needed.
 * This function also performs dead queue management, if the previous running thread
 * is marked as DEAD, it is removed from the dead queue and its resources are freed.
 */
void schedule_thread(void);

/*
 * Updates the scheduler state and re-schedule threads if needed.
 */
void update_schedule(void);

/*
 * Suspends the currently running thread voluntarily.
 * The thread is moved to the ready queue and marked as READY.
 * The thread will be scheduled again by the scheduler when it is its turn.
 *
 * RETURNS
 * TRUE on success
 * FALSE on failure
 */
BOOL scheduler_suspend_thread(void);

/*
 * Resumes a thread that was waiting on an event, a barrier, an I/O wait or a poll.
 * The thread is identified by its TID and the flags that caused it to wait.
 * The thread is flagged as READY and moved to the ready queue by update_schedule().
 * update_schedule() is not invoked directly by this procedure, but the thread will
 * be resumed at the next call to schedule_thread()/update_schedule().
 *
 * PARAMETERS IN
 * uint32_t flags - the flags that caused the thread to wait, this is used to check if the thread is waiting for the event that is being signaled.
 * uint8_t  tid   - the TID of the thread to be resumed.
 *
 * RETURNS
 * TRUE on success
 * FALSE on failure
 */
BOOL scheduler_resume_thread(uint32_t flags, uint8_t tid);

/*
 * Delays the currently running thread for a specified number of milliseconds.
 * The thread is moved to the delayed queue and marked as DELAYED.
 * The thread will be resumed at the end of the delay period.
 *
 * PARAMETERS IN
 * uint64_t msecs - the number of milliseconds to delay the thread.
 *
 * RETURNS
 * TRUE on success
 * FALSE on failure
 */
BOOL scheduler_delay_thread(uint64_t msecs);

/*
 * Waits for a specific event or condition to occur.
 * The thread is moved to the wait queue and marked as WAITING.
 * The thread will be resumed when the event occurs or when a timeout occurs.
 *
 * PARAMETERS IN
 * uint32_t flags - the flags that define what event or condition to wait for.
 * uint64_t msecs - the number of milliseconds to wait before timing out (0 means no timeout).
 *
 * RETURNS
 * TRUE on success (thread was added to wait queue)
 * FALSE on failure (thread was not added to wait queue)
 */
BOOL scheduler_wait_thread(uint32_t flags, uint64_t msecs);

/*
 * Adds a new thread to the scheduler's ready queues.
 *
 * PARAMETERS IN
 * uint8_t tid - the TID of the thread to be added.
 *
 * RETURNS
 * TRUE on success
 * FALSE on failure
 */
BOOL scheduler_add_thread(uint8_t tid);

/*
 * Removes a thread from the scheduler's queues and marks it as DEAD.
 * The thread will be moved to the dead queue and its resources will be freed by the scheduler.
 * If the thread was waiting on an event, a barrier, an I/O wait or a poll, it will be removed
 * from the wait queue and its flags will be cleared.
 * This function is a cancellation point.
 *
 * PARAMETERS IN
 * uint8_t tid - the TID of the thread to be removed.
 *
 * RETURNS
 * TRUE on success
 * FALSE on failure
 */
BOOL scheduler_remove_thread(uint8_t tid);

/*
 * Returns the TID of the currently running thread.
 *
 * RETURNS
 * the TID of the currently running thread, or THREAD_TID_INVALID if no thread is running.
 */
uint8_t scheduler_running_tid(void);

/*
 * Returns a pointer to the running thread's context.
 *
 * RETURNS
 * a pointer to the running thread's context, or NULL if no thread is running.
 */
thread_t *scheduler_running_thread(void);

/*
 * Returns the number of threads in the ready queues with priority higher than the specified one.
 *
 * PARAMETERS IN
 * uint8_t prio - the priority level to check, threads with priority lower than this will be counted.
 *
 * RETURNS
 * the number of threads in the ready queues with priority higher than the specified one.
 */
uint8_t scheduler_ready_threads(uint8_t prio);

/*
 * This is a default fail-safe function which is set as final return point for all threads,
 * if a thread returns from its entry point, this function will be called and will panic the kernel.
 * This is a safety measure to prevent threads from returning and causing undefined behavior.
 * This function should never be called, if it is called, it means that a thread has returned
 * from its entry point without calling thread_terminate(), which is a bug in the thread's code.
 */
void scheduler_fail_safe(void);

/*
 * Dumps the scheduler state, including the running thread, the ready queues, the delayed queue,
 * the wait queue and the dead queue.
 * This function is useful for debugging purposes and can be called from a thread or from
 * an interrupt context.
 * The output is printed to the console.
 */
void scheduler_dump(void);

#endif // SCHEDULER_H_INCLUDED
