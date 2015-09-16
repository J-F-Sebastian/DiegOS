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

#ifndef _EVENTS_PRIVATE_H_
#define _EVENTS_PRIVATE_H_

/*
 * Initialize the events library.
 * Must be called internally by kernel init routine.
 *
 * RETURN VALUES
 * TRUE if initialization succeded.
 * FALSE in any other case
 */
BOOL init_events_lib(void);

/*
 * Resume threads waiting for an event and put them
 * into the ready queue. This function must be called
 * internally by the scheduler when a new running thread
 * has to be selected.
 * The function will loop all the events' queue, if the
 * queue size is not 0, the thread waiting for the queue
 * will be set to READY state and moved into the ready queue.
 */
void resume_on_events(void);

#endif

