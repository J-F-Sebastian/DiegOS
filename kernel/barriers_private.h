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
BOOL init_barriers_lib (void);

/*
 * Resume any waiting thread on open barriers.
 * Must be called internally by the scheduler.
 * The routine loops all barriers, open barriers are scanned for pending
 * (waiting) threads, if any, they are resumed.
 */
void resume_on_barriers (void);

#endif

