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

#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <libs/queue_type.h>

typedef void (*event_freefn) (void *ptr);

typedef struct event {
	queue_node header;
	uint16_t classid;
	uint16_t eventid;
	event_freefn freefn;
} event_t;

typedef struct ev_queue ev_queue_t;

/*
 * Events queue API.
 * Events are messages stored in a queue. One thread only can be listening
 * to the events' queue; when at least one event message is in the queue, the
 * listener is woken up.
 * Most common step to use events queue are:
 *
 * 1) Create a queue by calling event_create_queue()
 * 2) set the running thread as a listener by calling event_watch_queue()
 * 3) Wait for events by calling wait_for_events()
 * 4) Get events by calling event_get(), objects are removed from the queue
 *
 * You eventually want to add messages to the queue by calling event_put().
 */

/*
 * Create an event queue providing a name (optional).
 *
 * PARAMETERS IN
 * const char *name - events' queue name, can be NULL
 *
 * RETURNS
 * A pointer to an events queue handle or NULL in case of failure.
 */
ev_queue_t *event_init_queue(const char *name);

/*
 * Terminates and destroys an event queue object.
 * Any pending events are dumped prior to effectively removing and releasing
 * the object.
 *
 * PARAMETERS IN
 * ev_queue_t *evqueue - pointer to a events queue object
 *
 * RETURNS
 * EINVAL in case evqueue is NULL
 * EGENERIC in case operations on the evqueue object fails (the object IS NOT
 *          removed or released)
 * EOK in case of success
 */
int event_done_queue(ev_queue_t * evqueue);

/*
 * Set the running thread as the listener thread for a specific events queue.
 * Once set, this is the only thread that will be processing messages stored
 * in the queue.
 *
 * PARAMETERS IN
 * ev_queue_t *evqueue - pointer to a events queue object
 *
 * RETURNS
 * EINVAL in case evqueue is NULL or the listener thread is already set
 * EOK in case of success
 */
int event_watch_queue(ev_queue_t * evqueue);

/*
 * Cancel the watch on the events queue for the specified thread id.
 *
 * PARAMETERS IN
 * ev_queue_t *evqueue - pointer to a events queue object
 * uint8_t tid        - thread id to be removed as listener
 *
 * RETURNS
 * EINVAL in case evqueue is NULL or the specified tid is not the listener
 * EOK in case of success
 */
int event_cancel_watch_queue(ev_queue_t * evqueue, uint8_t tid);

/*
 * Returns the number of messages (events) stored in the queue.
 *
 * PARAMETERS IN
 * ev_queue_t *evqueue - pointer to a events queue object
 *
 * RETURNS
 * The number of events in queue.
 */
unsigned event_queue_size(ev_queue_t * evqueue);

/*
 * Put into the queue a new event. 
 * Caller is in charge of memory management,
 * events allocation/deallocation is not directly handled by this API.
 * The callback passed in fncb will be invoked by event_free() to release
 * allocated memory; if the parameter is set to NULL no action will
 * be performed.
 *
 * PARAMETERS IN
 * ev_queue_t *evqueue - pointer to a events queue object
 * event_t *ev         - pointer to the new event to be queued
 * event_freefn fncb   - function pointer to release memory
 *
 * RETURNS
 * EINVAL if parameters are not valid
 * EPERM if the operation is not allowed or queuing failed
 * EOK in case of success
 */
int event_put(ev_queue_t * evqueue, event_t * ev, event_freefn fncb);

/*
 * Get a new event from the queue.
 * Caller is in charge of memory management,
 * events allocation/deallocation is not directly handled by this API.
 * Call event_free() to eventually release memory after having
 * used the event.
 *
 * PARAMETERS IN
 * ev_queue_t *evqueue - pointer to a events queue object
 *
 * RETURNS
 * A valid pointer to an event or NULL in case of failure
 */
event_t *event_get(ev_queue_t * evqueue);

/*
 * Release memory allocated to ev pointer.
 * If the event structure has a release function pointer set to
 * NULL, no action is performed.
 * If the release function pointer is set, the callback fuction
 * is invoked with the ev pointer passed as parameter.
 *
 * PARAMETERS IN
 * event_t *ev        - pointer to an event
 *
 */
void event_free(event_t * ev);
/*
 * Suspend thread execution until one or more events are stored in the queue.
 * After at least one event is queued, the listening thread is woken up.
 * Suspension is NOT timed.
 *
 * PARAMETERS IN
 * ev_queue_t *evqueue - pointer to a events queue object
 *
 */
void wait_for_events(ev_queue_t * evqueue);

/*
 * Print to stderr the specified events queue's configuration and status.
 * If evqueue is null, all queues are processed.
 *
 * PARAMETERS IN
 * ev_queue_t *evqueue - a specific events queue to be dumped,
 *                       or NULL to dump'em all
 */
void event_dump(ev_queue_t * evqueue);

#endif
