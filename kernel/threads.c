/*******************************************
 *********** SOURCE FILE *******************
 *******************************************
 *
 * Name:
 * Author:
 * Date:
 *******************************************
 * Description:
 *
 *******************************************/

#include <stdlib.h>
#include <string.h>

#include "platform_include.h"
#include "threads.h"
#include "scheduler.h"
#include "fail_safe.h"
#include "kprintf.h"

static thread_t *thread_storage = NULL;
static void *context_storage = NULL;

static unsigned thread_num = 0;
static unsigned thread_max = 0;

BOOL init_thread_lib()
{
    unsigned i;

    if (DIEGOS_MAX_THREADS >= THREAD_TID_INVALID) {
        return (FALSE);
    }

    thread_max = DIEGOS_MAX_THREADS;

    thread_storage = (thread_t *)malloc(thread_max*sizeof(*thread_storage) +
                                        CACHE_ALN);
    context_storage = malloc(thread_max*MULTC(THREAD_CONTEXT_SIZE) + CACHE_ALN);

    /*
     * Yes we should release memory, but kernel is gonna panic so... who cares?
     */
    if (!thread_storage || !context_storage) {
        return (FALSE);
    }

    /*
     * We are loosing pointers here, it is voluntary as no one will ever
     * claim memory back, unless you reboot/reset/reload, and in that case
     * anything will be cleared out.
     */
    thread_storage = (thread_t *)ALNC((uintptr_t)thread_storage);
    context_storage = (void *)ALNC((uintptr_t)context_storage);

    memset(thread_storage, 0, thread_max*sizeof(*thread_storage));
    memset(context_storage, 0, thread_max*MULTC(THREAD_CONTEXT_SIZE));

    for (i = 0; i < thread_max; i++) {
        thread_storage[i].tid = THREAD_TID_INVALID;
        thread_storage[i].context = (context_storage +
                                     i*MULTC(THREAD_CONTEXT_SIZE));
    }

    return (TRUE);
}

uint8_t init_thread(const char *name,
                    uint8_t prio,
                    void (*entry_ptr)(void),
                    void *stack,
                    uint32_t stack_size)
{
    uint32_t i,new_tid = THREAD_TID_INVALID;

    if ((thread_max == thread_num) ||
            (!name) ||
            (!entry_ptr) ||
            !(prio < THREAD_PRIORITIES)) {
        return THREAD_TID_INVALID;
    }

    /*
     * Double function : trace first available slot,
     * seek no other processes have his name.
     * Do NOT bail out once an empty slot is found, need to go all
     * way down to thread_max to check all names.
     */
    for (i = 0; i < thread_max; i++) {
        if ((new_tid == THREAD_TID_INVALID) &&
                !thread_storage[i].name[0] &&
                (thread_storage[i].tid == THREAD_TID_INVALID)) {
            new_tid = i;
        } else if (!strncmp(thread_storage[i].name, name, THREAD_NAME_MAX)) {
            return THREAD_TID_INVALID;
        }
    }

    /*
     * Stack check ...
     */
    if (stack) {
        thread_storage[new_tid].stack = stack;
    } else {
        thread_storage[new_tid].stack = malloc(stack_size);
        thread_storage[new_tid].flags |= THREAD_FLAG_REL_STACK;
    }

    if (!thread_storage[new_tid].stack) {
        thread_storage[new_tid].flags = 0;
        return THREAD_TID_INVALID;
    }

    thread_storage[new_tid].stack_size = stack_size;

    /* Mark stack space for tracing */
    memset(thread_storage[new_tid].stack, 0xdf, stack_size);

    /*
     * OK GO!
     */
    strncpy(thread_storage[new_tid].name, name, THREAD_NAME_MAX);
    thread_storage[new_tid].name[THREAD_NAME_MAX] = 0;
    thread_storage[new_tid].priority = prio;
    thread_storage[new_tid].state = THREAD_READY;
    thread_storage[new_tid].tid = new_tid;
    thread_storage[new_tid].entry_ptr = entry_ptr;

    thread_num++;

    setup_context(thread_storage[new_tid].stack + stack_size,
                  scheduler_fail_safe,
                  thread_storage[new_tid].entry_ptr,
                  thread_storage[new_tid].context);

    kmsgprintf("Created thread %s TID %u Stack at %p\n",
               thread_storage[new_tid].name,
               new_tid,
               thread_storage[new_tid].stack);
    return (new_tid);
}

BOOL done_thread(uint8_t tid)
{
    thread_t *th = get_thread(tid);

    if (th) {
        if (th->flags & THREAD_FLAG_REL_STACK) {
            free(th->stack);
        }
        memset(th,0,sizeof(*th));
        th->tid = THREAD_TID_INVALID;
        --thread_num;
        kprintf("TID %u killed\n", tid);
        return (TRUE);
    }

    return (FALSE);
}

thread_t *get_thread(uint8_t tid)
{
    if ((tid < THREAD_TID_INVALID) &&
            (tid < thread_max) &&
            (thread_storage[tid].tid == tid)) {
        return (&thread_storage[tid]);
    }

    return (NULL);
}

const char *state2str(uint8_t state)
{
    switch (state) {
    case THREAD_RUNNING:
        return ("RUNNING");
    case THREAD_WAITING:
        return ("WAITING");
    case THREAD_READY:
        return ("READY");
    case THREAD_DEAD:
        return ("DEAD");
    }

    return ("N/A");
}

void check_thread_stack()
{
    unsigned i,j;
    int *stack_top;

    for (i = 0; i < thread_max; i++) {
        if (thread_storage[i].tid == THREAD_TID_INVALID) {
            continue;
        }
        j = 0;
        stack_top = (int *)(thread_storage[i].stack);
        while ((int)0xdfdfdfdf == *stack_top++) {
            ++j;
        }
        if (!j) {
            kerrprintf("Thread %u [%s] stack overflow !!!\n",
                       i,
                       thread_storage[i].name);
        } else {
            kprintf("Thread %u [%s] stack free: %u Bytes\n",
                    i,
                    thread_storage[i].name,
                    j*4);
        }
    }
}
