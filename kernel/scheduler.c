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

#include <stdio.h>
#include <errno.h>
#include <diegos/interrupts.h>

#include "threads.h"
#include "clock.h"
#include "scheduler.h"
#include "kprintf.h"
#include "events_private.h"
#include "barriers_private.h"

/*
 * various queues and pointers for thread
 * execution management.
 */

static thread_t *running = NULL;

/*
 * Ready queues must be protected from interrupts
 * routines handling events.
 * Events can wake up threads putting them into
 * one of the ready queues.
 * No other state transition is allowed from interrupt
 * context.
 */
static queue_inst ready_queues[THREAD_PRIORITIES];

static queue_inst dead_queue = {
    .head = NULL,
    .tail = NULL,
    .counter = 0
};

static queue_inst delay_queue = {
    .head = NULL,
    .tail = NULL,
    .counter = 0
};

static uint64_t delays[DIEGOS_MAX_THREADS];

/*
 * Perform house keeping, destroying dead threads waiting
 * in the dead queue (the green mile !).
 * The running thread might exit or call thread_terminate(),
 * so if it is the only one in the queue, just skip and go.
 * Remember that the running thread CANNOT kill itself in its
 * own context, the thread might have malloc'd the stack memory
 * and freeing it would crash the system.
 * If more than one thread is in the queue, dequeue and queue again
 * the running one, so a single loop can definitively get rid of
 * all non running threads at once.
 */

static inline BOOL should_do_house_keeping(void)
{
    uint32_t temp = queue_count(&dead_queue);

    if ((1 == temp) && (queue_head(&dead_queue) == running)) {
        return (FALSE);
    }

    return ((temp) ? (TRUE) : (FALSE));
}

static inline void house_keeping(void)
{
    thread_t *temp = NULL;
    unsigned i = queue_count(&dead_queue);

    while (i &&
            (EOK == queue_dequeue(&dead_queue,
                                  (queue_node **)&temp))) {

        if (temp->tid != running->tid) {
            if (!done_thread(temp->tid)) {
                kerrprintf("could not kill tid %d\n",temp->tid);
            }
        } else {
            if (EOK != queue_enqueue(&dead_queue, &temp->header)) {
                kerrprintf("tid %d is lost dead!\n",temp->tid);
            }
        }
        --i;
    }
}

static inline BOOL new_runner(unsigned q)
{
    thread_t *temp = NULL;

    while (EOK == queue_dequeue(&ready_queues[q],
                                (queue_node **)&temp)) {

        if (temp->flags & THREAD_FLAG_TERMINATE) {

            if (EOK != queue_enqueue(&dead_queue, &temp->header)) {
                kerrprintf("failed killing PID %d\n",temp->tid);
            } else {
                temp->state = THREAD_DEAD;
            }
        } else {
            running = temp;
            running->state = THREAD_RUNNING;
            //kprintf("Scheduled PID %d\n",temp->tid);
            return (TRUE);
        }
    }

    kerrprintf("could not schedule PRIO %d",q);

    return (FALSE);
}

/*
 * Sleeping threads not to be woken up are queued
 * again and again - silly, unefficient, cache miss prone.
 * But reuses the queue data structure and behavior of
 * all other scheduler features.
 */

static inline void schedule_delayed(void)
{
    uint64_t ticks = clock_get_ticks();
    thread_t *ptr = queue_head(&delay_queue);

    while (delays[ptr->tid] <= ticks) {
        if (EOK != queue_dequeue(&delay_queue, (queue_node **)&ptr)) {
            if (ptr) {
                kerrprintf("failed resuming delayed PID %u\n", ptr->tid);
            }
            /*
             * Break the loop as much probably the data structure is
             * corrupted, probably an assert or abort would be better
             * since the existence of other delayed threads will bring
             * the scheduler here in the future....
             */
            break;
        } else {
            (void)scheduler_resume_thread(THREAD_FLAG_DELAYED, ptr->tid);
            delays[ptr->tid] = 0;
        }
        ptr = queue_head(&delay_queue);
    }
}

void schedule_thread()
{
    unsigned i = 0;

    /*
     * Dead queue management
     */
    if (should_do_house_keeping()) {
        house_keeping();
    }

    /*
     * delayed queue management
     */
    if (queue_count(&delay_queue)) {
        schedule_delayed();
    }

    /*
     * Ready queue management (dead queue too again)
     */
    resume_on_events();
    resume_on_barriers();

    while (i < NELEMENTS(ready_queues)) {
        if (queue_count(&ready_queues[i])) {
            if (new_runner(i)) {
                return;
            }
        }
        ++i;
    }
}

void update_schedule()
{
    /*
     * delayed queue management
     */
    if (queue_count(&delay_queue)) {
        schedule_delayed();
    }

    /*
     * Ready queue management
     */
    resume_on_events();
    resume_on_barriers();
}

BOOL scheduler_init()
{
    unsigned i;

    for (i = 0; i < NELEMENTS(ready_queues); i++) {
        if (EOK != queue_init(&ready_queues[i])) {
            return (FALSE);
        }
    }

    if (EOK != queue_init(&delay_queue)) {
        return (FALSE);
    }

    if (EOK != queue_init(&dead_queue)) {
        return (FALSE);
    }

    return (TRUE);
}

BOOL scheduler_suspend_thread()
{
    if (EOK != queue_enqueue(&ready_queues[running->priority],
                             &running->header)) {
        kerrprintf("failed suspending PID %d\n",running->tid);
        return (FALSE);
    }

    running->state = THREAD_READY;

    return (TRUE);
}

BOOL scheduler_resume_thread(uint8_t flags, uint8_t tid)
{
    thread_t *ptr = get_thread(tid);

    if (!ptr) {
        kerrprintf("No process with TID %d\n",tid);
        return (FALSE);
    }

    if (THREAD_WAITING != ptr->state) {
        kerrprintf("TID %d is %s, cannot resume\n",tid,state2str(ptr->state));
        return (FALSE);
    }

    if (flags != (ptr->flags & THREAD_MASK_WAIT)) {
        kerrprintf("TID %d flags 0x%X mismatches flags 0x%X\n",
                   tid, ptr->flags & THREAD_MASK_WAIT, flags);
        return (FALSE);
    }

    if (EOK != queue_enqueue(&ready_queues[ptr->priority],
                             &ptr->header)) {
        kerrprintf("failed resuming TID %d\n",tid);
        return (FALSE);
    }

    ptr->flags &= ~flags;
    ptr->state = THREAD_READY;

    return (TRUE);
}

BOOL scheduler_delay_thread(uint64_t ticks)
{
    thread_t *ptr, *ptr2;

    if (THREAD_TID_IDLE == running->tid) {
        return (FALSE);
    }

    ptr = queue_head(&delay_queue);
    if (!ptr || (ticks < delays[ptr->tid])) {
        if (EOK != queue_insert(&delay_queue,
                                &running->header,
                                NULL)) {
            kerrprintf("failed delaying PID %d\n",running->tid);
            return (FALSE);
        }
    } else {
        while (ptr->header.next) {
            ptr2 = (thread_t *)ptr->header.next;
            if (ticks < delays[ptr2->tid]) {
                break;
            }
            ptr = (thread_t *)ptr->header.next;
        }
        if (EOK != queue_insert(&delay_queue,
                                &running->header,
                                &ptr->header)) {
            kerrprintf("failed delaying PID %d\n",running->tid);
            return (FALSE);
        }
    }

    running->flags |= THREAD_FLAG_DELAYED;
    running->state = THREAD_WAITING;
    delays[running->tid] = ticks;

    return (TRUE);
}

BOOL scheduler_wait_thread(uint8_t flags)
{
    const uint8_t MYMASK = THREAD_FLAG_WAIT_MUTEX |
                           THREAD_FLAG_WAIT_EVENT |
                           THREAD_FLAG_WAIT_BARRIER |
                           THREAD_FLAG_WAIT_COMPLETION;

    if (THREAD_TID_IDLE == running->tid) {
        return (FALSE);
    }

    if (!(MYMASK & flags)) {
        kerrprintf("Unknown flag 0x%X\n", flags);
        return (FALSE);
    }

    /*kprintf("Stopping running PID %d\n",running->tid);*/
    running->flags |= flags;
    running->state = THREAD_WAITING;

    return (TRUE);
}

BOOL scheduler_add_thread(uint8_t tid)
{
    STATUS retcode;
    thread_t *ptr = get_thread(tid);

    if (!ptr || (THREAD_READY != ptr->state)) {
        return (FALSE);
    }

    retcode = queue_enqueue(&ready_queues[ptr->priority],
                            &ptr->header);

    if (EOK != retcode) {
        kerrprintf("failed scheduling PID %d\n",tid);
    }

    return ((EOK == retcode) ? (TRUE) : (FALSE));
}

BOOL scheduler_remove_thread(uint8_t tid)
{
    thread_t *ptr = get_thread(tid);

    if (!ptr) {
        return (FALSE);
    }

    if (THREAD_TID_IDLE == tid) {
        kerrprintf("No removal for IDLE\n");
        return (FALSE);
    }

    switch (ptr->state) {
        /*
         * RUNNING and WAITING threads are not linked in a ready queue
         */
        case THREAD_RUNNING:
            ptr->state = THREAD_DEAD;
            if (EOK != queue_enqueue(&dead_queue, &ptr->header)) {
                kerrprintf("failed killing PID %d\n",tid);
                return (FALSE);
            }
            break;

        case THREAD_WAITING:
            ptr->state = THREAD_DEAD;
            if (EOK != queue_enqueue(&dead_queue, &ptr->header)) {
                kerrprintf("failed killing PID %d\n",tid);
                return (FALSE);
            }
            break;
        /*
         * READY threads are linked in a ready queue, so flag them and remove
         * them when executing new_runner()
         */
        case THREAD_READY:
            ptr->flags |= THREAD_FLAG_TERMINATE;
            break;

        /*
         * A DEAD thread is already dead....
         */
        case THREAD_DEAD:
            kprintf("I am DEAD !!!\n");
            return (FALSE);
    }

    return (TRUE);
}

uint8_t scheduler_running_tid()
{
    return ((running) ? (running->tid) : (THREAD_TID_INVALID));
}

uint8_t scheduler_ready_threads(uint8_t prio)
{
    unsigned i = 0;
    uint8_t total = 0;

    while (i < prio) {
        total += queue_count(&ready_queues[i++]);
    }

    return (total);
}

thread_t *scheduler_running_thread()
{
    return (running);
}

