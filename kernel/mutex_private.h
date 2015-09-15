#ifndef MUTEX_H_INCLUDED
#define MUTEX_H_INCLUDED

/*******************************************
 *********** HEADER FILE *******************
 *******************************************
 *
 * Name:
 * Author:
 * Date:
 *******************************************
 * Description: KERNEL INTERNALS
 *
 *******************************************/

#include <types_common.h>
#include <queue_type.h>
#include <list_type.h>

#include "mutex_data.h"

/*
 * Initialize the mutex library.
 * This must be called internally by the kernel init routine.
 *
 * RETURN VALUES
 * TRUE if initialization succeded
 * FALSE in any other case.
 */
BOOL init_mutex_lib(void);

/*
 * Initialize a new mutex.
 * PARAMETERS IN
 * const char *name - name of the mutex, optional. If no name is provided,
 *                    a default willb e generated.
 * RETURN VALUES
 * TRUE success.
 * FALSE any other case.
 */
struct mutex *init_mutex(const char *name);

/*
 * Terminate a mutex.
 * PARAMETERS IN
 * struct mutex *mtx - the mutex to be locked.
 * RETURN VALUES
 * TRUE success.
 * FALSE any other case.
 */
BOOL done_mutex(struct mutex *mtx);

/*
 * Acquire a lock on a mutex. Once a thread acquired a lock,
 * other threads trying the same will be put in a wait FIFO queue.
 * A thread can call lock_mutex multiple times before releasing the lock;
 * calls other than the first are no-op.
 * PARAMETERS IN
 * uint8_t tid - the TID of the locking thread.
 * struct mutex *mtx - the mutex to be locked.
 */
BOOL lock_mutex(uint8_t tid, struct mutex *mtx);

/*
 * Release a lock on a mutex. When a thread releases a lock, the first
 * thread found in the waiting queue is woken up and given a new lock
 * on the mutex.
 * If the waiting queue is empty, the mutex is unlocked.
 * PARAMETERS IN
 * uint8_t tid - the TID of the locking thread.
 * struct mutex *mtx - the mutex to be locked.
 */
BOOL unlock_mutex(uint8_t tid, struct mutex *mtx);

/*
 * Test if a mutex is locked. this function should be used to poll a mutex
 * status without incurring in suspension.
 * RETURN VALUES
 * TRUE the mutex is locked.
 * FALSE the mutex is unlocked.
 */
BOOL mutex_is_locked(struct mutex *mtx);

void dump_mutex (struct mutex *mtx);

#endif // MUTEX_H_INCLUDED
