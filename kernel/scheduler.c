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
#include <diegos/kernel_ticks.h>

#include "threads.h"
#include "clock.h"
#include "scheduler.h"
#include "kprintf.h"
#include "events_private.h"
#include "barriers_private.h"
#include "io_waits_private.h"
#include "poll_private.h"
#include "mutex_private.h"

/*
 * Maximum delay for the system clock tick.
 * The scheduler will not delay more than this value
 * to re-awake delayed threads.
 * Sleeping threads are checked at every call to schedule_thread().
 * If a thread is delayed for more than SCHED_DELAY_MAX, the scheduler
 * will re-check the delayed queue after SCHED_DELAY_MAX milliseconds or less.
 * This avoids problems with very long delays and clock overflows.
 * The scheduler clock period is somewhat quantized.
 * The value is in milliseconds.
 */
#define SCHED_DELAY_MAX (10*1000UL)

/*
 * Pointer to the currently running thread.
 * Only one thread can be running at a time.
 * The scheduler does not support multiple processors yet.
 */
static thread_t *running = NULL;

/*
 * Ready queues for each priority level.
 * All threads in these queues are in READY state.
 * Transition to or from this list must not happen
 * while interrupts are disabled.
 * State transitions must not be performed while in an interrupt context.
 */
static queue_inst ready_queues[THREAD_PRIORITIES];

/*
 * Dead queue for threads that have terminated.
 * Threads in this queue are waiting for the scheduler
 * to destroy them.
 * Transition to DEAD state must not happen while interrupts are disabled.
 * A RUNNING thread can transition into this queue as well as a WAITING thread.
 * A READY thread cannot transition into this queue.
 * A RUNNING thread can terminate itself, but the actual destruction
 * of the thread context and stack must be performed by the scheduler.
 * A WAITING thread can be terminated by another thread.
 */
static queue_inst dead_queue = {
	.head = NULL,
	.tail = NULL,
	.counter = 0
};

/*
 * Delayed queue for threads that are sleeping.
 * Threads in this queue are waiting for a delay to expire.
 * Transition from/to DELAYED state must not happen while interrupts are disabled.
 */
static queue_inst delay_queue = {
	.head = NULL,
	.tail = NULL,
	.counter = 0
};

/*
 * Wait queue for threads that are waiting on events or a timeout.
 * Threads in this queue are waiting for a delay to expire or for an alarm to
 * trigger or for I/O to complete with timeout.
 * Transition from/to WAITING state must not happen while interrupts are disabled.
 */
static queue_inst wait_queue = {
	.head = NULL,
	.tail = NULL,
	.counter = 0
};

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

/*
 * Empties the dead queue.
 */
static inline void house_keeping(void)
{
	thread_t *temp = NULL;
	unsigned i = queue_count(&dead_queue);

	while (i && (EOK == queue_dequeue(&dead_queue, (queue_node **) & temp))) {

		if (temp->tid != running->tid) {
			if (!done_thread(temp->tid)) {
				kerrprintf("could not kill tid %d\n", temp->tid);
			}
		} else {
			if (EOK != queue_enqueue(&dead_queue, &temp->header)) {
				kerrprintf("tid %d is lost dead!\n", temp->tid);
			}
		}
		--i;
	}
}

/*
 * Select a new running thread from the ready queue.
 * At the same time moves dead threads to the dead queue.
 */
static inline BOOL new_runner(unsigned q)
{
	thread_t *temp = NULL;

	while (EOK == queue_dequeue(&ready_queues[q], (queue_node **) & temp)) {

		if (temp->flags & THREAD_FLAG_TERMINATE) {

			if (EOK != queue_enqueue(&dead_queue, &temp->header)) {
				kerrprintf("failed killing PID %d\n", temp->tid);
			} else {
				temp->state = THREAD_DEAD;
			}
		} else {
			running = temp;
			running->state = THREAD_RUNNING;
			return (TRUE);
		}
	}

	kerrprintf("could not schedule PRIO %d", q);

	return (FALSE);
}

/*
 * Check the wait queue for threads whose delay has expired and are still waiting
 * and resume them.
 * Check also for threads that became ready and move the to the ready qeues.
 * It is not assumed that threads in the wait queue are sorted by delay
 * expiration time.
 * Also, the threads in the wait queue have their delay field set
 * to the absolute expiration time (in milliseconds since boot).
 */
static void schedule_waiting(void)
{
	uint64_t expiration = clock_get_milliseconds();
	thread_t *ptr, *temp;
	unsigned i;

	i = queue_count(&wait_queue);
	while (i--) {
		ptr = queue_head(&wait_queue);

		switch (ptr->state) {
		case THREAD_WAITING:
			if (ptr->flags & THREAD_FLAG_TERMINATE) {
				if (EOK != queue_dequeue(&wait_queue, (queue_node **) & temp)) {
					kerrprintf("failed extracting TID %d from wait queue\n",
						   ptr->tid);
				} else if (EOK != queue_enqueue(&dead_queue, &temp->header)) {
					kerrprintf("failed killing PID %d\n", temp->tid);
				} else {
					temp->state = THREAD_DEAD;
				}
			} else if ((ptr->flags & THREAD_FLAG_WAIT_TIMEOUT)
				   && (ptr->delay <= expiration)) {
				if (EOK != queue_dequeue(&wait_queue, (queue_node **) & temp)) {
					kerrprintf("failed extracting TID %d from wait queue\n",
						   ptr->tid);
				} else {
					if (!scheduler_resume_thread
					    (temp->flags & THREAD_MASK_EVENTS, temp->tid)) {
						kerrprintf("failed resuming TID %d\n", temp->tid);
					} else if (EOK !=
						   queue_enqueue(&ready_queues[temp->priority],
								 &temp->header)) {
						kerrprintf("failed moving TID %d to ready queue\n",
							   temp->tid);
					}
				}
			}
			break;

		case THREAD_READY:
			if (EOK != queue_dequeue(&wait_queue, (queue_node **) & temp)) {
				kerrprintf("failed extracting TID %d from wait queue\n", ptr->tid);
			} else if (EOK !=
				   queue_enqueue(&ready_queues[temp->priority], &temp->header)) {
				kerrprintf("failed moving TID %d to ready queue\n", temp->tid);
			}
			break;
		}

		if (EOK != queue_roll(&wait_queue)) {
			kerrprintf("failed rolling wait queue\n");
			return;
		}
	}
}

/*
 * Check the delayed queue for threads whose delay has expired
 * and resume them.
 * It is assumed that threads in the delay queue are sorted by delay
 * expiration time, so the head of the queue is always the next
 * thread to be resumed.
 * Also, the threads in the delay queue have their delay field set
 * to the absolute expiration time (in milliseconds since boot).
 */
static void schedule_delayed(void)
{
	uint64_t expiration = clock_get_milliseconds();
	thread_t *ptr = queue_head(&delay_queue);
	thread_t *temp;

	while (ptr && (ptr->delay <= expiration)) {
		if (EOK != queue_dequeue(&delay_queue, (queue_node **) & temp)) {
			kerrprintf("failed resuming delayed PID %u\n", ptr->tid);
			/*
			 * Break the loop as much probably the data structure is
			 * corrupted, probably an assert or abort would be better
			 * since the existence of other delayed threads will bring
			 * the scheduler here in the future....
			 */
			break;
		} else {
			if (!scheduler_resume_thread(THREAD_FLAG_WAIT_TIMEOUT, temp->tid)) {
				kerrprintf("failed resuming delayed TID %d\n", temp->tid);
			} else if (EOK !=
				   queue_enqueue(&ready_queues[temp->priority], &temp->header)) {
				kerrprintf("failed moving delayed TID %d to ready queue\n",
					   temp->tid);
			}
		}
		ptr = queue_head(&delay_queue);
	}
}

/*
 * Peek at the top expiration time in the delay queue.
 * In case the delay is longer than SCHED_DELAY_MAX,
 * return SCHED_DELAY_MAX.
 */
static inline uint32_t peek_top_expiration(void)
{
	thread_t *ptr = queue_head(&delay_queue);
	uint64_t now = clock_get_milliseconds();
	uint64_t retval;

	retval = (ptr->delay > now) ? (ptr->delay - now) : 0;
	return (retval < SCHED_DELAY_MAX) ? (uint32_t) retval : SCHED_DELAY_MAX;
}

void schedule_thread()
{
	unsigned i = 0;
	uint32_t new_delay = SCHED_DELAY_MAX;

	/*
	 * Wait queue management, update waiting threads state
	 */
	resume_on_events();
	resume_on_barriers();
	resume_on_io_wait();
	resume_on_poll();

	/*
	 * Wait queue management, fill in the ready queues if needed
	 */
	if (queue_count(&wait_queue)) {
		schedule_waiting();
		//if (queue_count(&wait_queue)) {
		//      new_delay = peek_top_expiration();
		//}
	}

	/*
	 * delayed queue management, fill in the ready queues if needed
	 */
	if (queue_count(&delay_queue)) {
		schedule_delayed();
		if (queue_count(&delay_queue)) {
			new_delay = peek_top_expiration();
		}
	}
	if (FALSE == clock_set_period(new_delay, CLK_INST_SCHEDULER)) {
		kerrprintf("Clock device failed in %s\n", __FUNCTION__);
	}

	while (i < NELEMENTS(ready_queues)) {
		if (queue_count(&ready_queues[i])) {
			if (new_runner(i)) {
				break;
			}
		}
		++i;
	}

	/*
	 * Dead queue management.
	 * Keep it here as the running thread might have called
	 * thread_terminate in which case it is not possible to
	 * destroy the thread in its own context, i.e. the running thread
	 * must change to destroy the previous running thread.
	 */
	if (should_do_house_keeping()) {
		house_keeping();
	}
}

void update_schedule()
{
	uint32_t new_delay = SCHED_DELAY_MAX;

	/*
	 * Wait queue management, update waiting threads state
	 */
	resume_on_events();
	resume_on_barriers();
	resume_on_io_wait();
	resume_on_poll();

	/*
	 * Wait queue management, fill in the ready queues if needed
	 */
	if (queue_count(&wait_queue)) {
		schedule_waiting();
	}

	/*
	 * delayed queue management, fill in the ready queues if needed
	 */
	if (queue_count(&delay_queue)) {
		schedule_delayed();
		if (queue_count(&delay_queue)) {
			new_delay = peek_top_expiration();
		}
	}
	if (FALSE == clock_set_period(new_delay, CLK_INST_SCHEDULER)) {
		kerrprintf("Clock device failed in %s\n", __FUNCTION__);
	}
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

	if (EOK != queue_init(&wait_queue)) {
		return (FALSE);
	}

	return (TRUE);
}

BOOL scheduler_suspend_thread()
{
	if (EOK != queue_enqueue(&ready_queues[running->priority], &running->header)) {
		kerrprintf("failed suspending PID %d\n", running->tid);
		return (FALSE);
	}

	running->state = THREAD_READY;

	return (TRUE);
}

BOOL scheduler_resume_thread(uint32_t flags, uint8_t tid)
{
	thread_t *ptr = get_thread(tid);
	uint32_t check;

	if (!ptr) {
		kerrprintf("No process with TID %d\n", tid);
		return (FALSE);
	}

	if (THREAD_WAITING != ptr->state) {
		kerrprintf("TID %d state %s flags %s, cannot resume\n", tid, state2str(ptr->state),
			   flags2str(ptr->flags));
		return (FALSE);
	}

	check = flags & (ptr->flags & THREAD_MASK_WAIT);
	if (check == 0) {
		kerrprintf("TID %d flags %x %s mismatches flags %x %s\n",
			   tid, ptr->flags & THREAD_MASK_WAIT,
			   flags2str(ptr->flags & THREAD_MASK_WAIT), flags, flags2str(flags));
		return (FALSE);
	}

	ptr->flags &= ~check;
	ptr->state = THREAD_READY;
	ptr->delay = 0;

	return (TRUE);
}

BOOL scheduler_delay_thread(uint64_t msecs)
{
	thread_t *ptr, *ptr2;

	if ((THREAD_TID_IDLE == running->tid) || (THREAD_TID_TMRS == running->tid)) {
		return (FALSE);
	}

	ptr = queue_head(&delay_queue);
	if (!ptr || (msecs < ptr->delay)) {
		if (EOK != queue_insert(&delay_queue, &running->header, NULL)) {
			kerrprintf("failed delaying PID %d\n", running->tid);
			return (FALSE);
		}
	} else {
		while (ptr->header.next) {
			ptr2 = (thread_t *) ptr->header.next;
			if (msecs < ptr2->delay) {
				break;
			}
			ptr = (thread_t *) ptr->header.next;
		}
		if (EOK != queue_insert(&delay_queue, &running->header, &ptr->header)) {
			kerrprintf("failed delaying PID %d\n", running->tid);
			return (FALSE);
		}
	}

	running->flags |= THREAD_FLAG_WAIT_TIMEOUT;
	running->state = THREAD_WAITING;
	running->delay = msecs;

	return (TRUE);
}

BOOL scheduler_wait_thread(uint32_t flags, uint64_t msecs)
{
	if (THREAD_TID_IDLE == running->tid) {
		return (FALSE);
	}

	if (!(THREAD_MASK_EVENTS & flags)) {
		kerrprintf("Unknown flag 0x%X\n", flags);
		return (FALSE);
	}

	if (EOK != queue_enqueue(&wait_queue, &running->header)) {
		kerrprintf("failed waiting PID %d\n", running->tid);
		return (FALSE);
	}

	flags &= THREAD_MASK_EVENTS;
	running->flags |= flags;
	running->state = THREAD_WAITING;
	running->delay = msecs;

	return (TRUE);
}

BOOL scheduler_add_thread(uint8_t tid)
{
	STATUS retcode;
	thread_t *ptr = get_thread(tid);

	if (!ptr || (THREAD_READY != ptr->state)) {
		return (FALSE);
	}

	retcode = queue_enqueue(&ready_queues[ptr->priority], &ptr->header);

	if (EOK != retcode) {
		kerrprintf("failed scheduling PID %d\n", tid);
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

	if (THREAD_TID_TMRS == tid) {
		kerrprintf("No removal for TIMERS\n");
		return (FALSE);
	}

	switch (ptr->state) {
		/*
		 * RUNNING thread is not linked in a ready queue
		 */
	case THREAD_RUNNING:
		ptr->state = THREAD_DEAD;
		if (EOK != queue_enqueue(&dead_queue, &ptr->header)) {
			kerrprintf("failed killing PID %d\n", tid);
			return (FALSE);
		}
		break;

	case THREAD_WAITING:
		switch (ptr->flags & THREAD_MASK_WAIT) {
		case THREAD_FLAG_WAIT_EVENT:
			if (EOK != cancel_wait_for_events(tid)) {
				kerrprintf("failed killing PID %d\n", tid);
				return (FALSE);
			}
			break;

		case THREAD_FLAG_WAIT_BARRIER:
			if (EOK != cancel_wait_for_barrier(tid)) {
				kerrprintf("failed killing PID %d\n", tid);
				return (FALSE);
			}
			break;

		case THREAD_FLAG_WAIT_COMPLETION:
			if (EOK != cancel_io_waits(tid)) {
				kerrprintf("failed killing PID %d\n", tid);
				return (FALSE);
			}
			break;

		case THREAD_FLAG_WAIT_TIMEOUT:
			break;

		case THREAD_FLAG_WAIT_MUTEX:
			/*
			 * Threads waiting on mutexes are stored in lists and queues,
			 * when cancelled are orphaned so we need to put them directly in the dead queue
			 * as if they were running threads.
			 */
			if (EOK != cancel_wait_on_mutex(tid)) {
				kerrprintf("failed killing PID %d\n", tid);
				return (FALSE);
			} else {
				ptr->state = THREAD_DEAD;
				if (EOK != queue_enqueue(&dead_queue, &ptr->header)) {
					kerrprintf("failed killing PID %d\n", tid);
					return (FALSE);
				}
			}
			break;
		}

		ptr->flags &= ~THREAD_MASK_WAIT;
		/* FALLTHRU */
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

void scheduler_dump()
{
	unsigned i;

	kprintf("=== SCHEDULER DUMP ===\n");
	kprintf("Running TID: %d\n", scheduler_running_tid());
	for (i = 0; i < NELEMENTS(ready_queues); i++) {
		kprintf("Ready queue PRIO %d: %d threads\n", i, queue_count(&ready_queues[i]));
		if (queue_count(&ready_queues[i])) {
			thread_t *ptr = queue_head(&ready_queues[i]);
			while (ptr) {
				kprintf("  TID %d NAME %15s STATE %s FLAGS %s\n", ptr->tid,
					ptr->name, state2str(ptr->state), flags2str(ptr->flags));
				ptr = (thread_t *) ptr->header.next;
			}
		}
	}
	kprintf("Delay queue: %d threads\n", queue_count(&delay_queue));
	if (queue_count(&delay_queue)) {
		thread_t *ptr = queue_head(&delay_queue);
		while (ptr) {
			kprintf("  TID %d NAME %15s STATE %s FLAGS %s\n", ptr->tid, ptr->name,
				state2str(ptr->state), flags2str(ptr->flags));
			ptr = (thread_t *) ptr->header.next;
		}
	}
	kprintf("Wait queue: %d threads\n", queue_count(&wait_queue));
	if (queue_count(&wait_queue)) {
		thread_t *ptr = queue_head(&wait_queue);
		while (ptr) {
			kprintf("  TID %d NAME %15s STATE %s FLAGS %s\n", ptr->tid, ptr->name,
				state2str(ptr->state), flags2str(ptr->flags));
			ptr = (thread_t *) ptr->header.next;
		}
	}
	kprintf("Dead queue: %d threads\n", queue_count(&dead_queue));
	kprintf("======================\n");
}
