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

#ifndef _BARRIERS_PRIVATE_H_
#define _BARRIERS_PRIVATE_H_

/*
 * Initialize the barriers library.
 * Must be called internally by the kernel init
 * routine.
 *
 * RETURN VALUES
 *
 * TRUE if initialization succeded
 * FALSE in any other case
 */
BOOL init_barriers_lib(void);

/*
 * Resume any waiting thread on open barriers.
 * Must be called internally by the scheduler.
 * The routine loops all barriers, open barriers are scanned for pending
 * (waiting) threads, if any, they are resumed.
 */
void resume_on_barriers(void);

/*
 * Cancel a wait for a barrier.
  *
 * PARAMETERS IN
  * uint8_t tid        - thread ID to cancel wait for
 *
 * RETURNS
 * EOK in case of success
 * EINVAL if barrier is invalid
 * EPERM if the thread wait cannot be cancelled
 */
int cancel_wait_for_barrier(uint8_t tid);

#endif
