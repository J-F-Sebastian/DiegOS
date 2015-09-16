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

#ifndef _FAIL_SAFE_H_
#define _FAIL_SAFE_H_

/*
 * A fail safe exit procedure. The kernel
 * will use this procedure as a return value for
 * every thread's entry point.
 * when a thread is spawn, the entry point (a 
 * function) should never return.
 * To cleanly terminate a thread of execution, 
 * the specific kernel call thread_terminate()
 * should be invoked.
 * In case this call is missing, the entry point
 * function would terminate AND return to the caller;
 * this action may result in loading the program counter
 * with bogus data from the stack or a memory registry.
 * To avoid this, a fail safe return value is stored as
 * the caller of any thread entry point.
 * This fall back procedure will print an error message and
 * terminate the thread.
 * PLEASE NOTE: do NOT rely on the fail safe to avoid calling
 * thread_terminate(). The fail safe procedure is NOT aware of
 * any managed objects the thread is working on, i.e. it will
 * not free malloc'd memory, it will not release mutexes, spinlocks,
 * semaphores, it won't close files, pipes, or free any other kind of
 * resource.
 * To cleanly terminate a thread of execution, PLEASE free your resources
 * and call thread_terminate().
 */
void scheduler_fail_safe(void);

#endif

