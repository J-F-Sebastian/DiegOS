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

