#ifndef THREADS_DATA_H_INCLUDED
#define THREADS_DATA_H_INCLUDED

/*******************************************
 *********** HEADER FILE *******************
 *******************************************
 *
 * Name:
 * Author:
 * Date:
 *******************************************
 * Description:
 *
 *******************************************/

#include <types_common.h>
#include <queue.h>

#define THREAD_TID_IDLE     (0)
#define THREAD_TID_INVALID  (255)
#define THREAD_NAME_MAX     (15)
#define THREAD_PRIORITIES   (4)
#define THREAD_DEF_STACK    (2*1024)

enum {
    THREAD_RUNNING,
    THREAD_WAITING,
    THREAD_READY,
    THREAD_DEAD
};

enum {
    THREAD_FLAG_TERMINATE       = 1 << 0,
    THREAD_FLAG_REL_STACK       = 1 << 1,
    THREAD_FLAG_DELAYED         = 1 << 2,
    THREAD_FLAG_WAIT_MUTEX      = 1 << 3,
    THREAD_FLAG_WAIT_EVENT      = 1 << 4,
    THREAD_FLAG_WAIT_BARRIER    = 1 << 5,
    THREAD_FLAG_WAIT_COMPLETION = 1 << 6,
    THREAD_MASK_WAIT            = (THREAD_FLAG_DELAYED      |
                                   THREAD_FLAG_WAIT_MUTEX   |
                                   THREAD_FLAG_WAIT_EVENT   |
                                   THREAD_FLAG_WAIT_BARRIER |
                                   THREAD_FLAG_WAIT_COMPLETION)
};

typedef struct thread {
    /*
     * Container structure is a single linked list (queue)
     */
    queue_node header;
    /*
     * The context of a thread is platform dependent,
     * the size has to be defined under platforms/<name>
     */
    void *context;
    /*
     * Thread name
     */
    char name[THREAD_NAME_MAX + 1];
    /*
     * Thread properties: priority, state machine's state,
     * flags (mainly for event wait loops), thread id.
     */
    uint32_t priority:4;
    uint32_t state:8;
    uint32_t flags:12;
    uint32_t tid:8;
    /*
     * Thread entry point, NO return value, NO parameters
     */
    void (*entry_ptr)(void);
    /*
     * Thread's own stack, this will be used by interrupts
     * if they happen while this thread is executing - there
     * is a minimum stack size requirement.
     */
    void *stack;
    uint32_t stack_size;
} thread_t;

#endif // THREADS_DATA_H_INCLUDED
