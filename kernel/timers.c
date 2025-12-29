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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <diegos/kernel.h>
#include <diegos/kernel_ticks.h>
#include <diegos/interrupts.h>
#include <diegos/barriers.h>
#include <diegos/timers.h>
#include <libs/list.h>

#include "timers_private.h"
#include "kernel_private.h"
#include "clock.h"
#include "kprintf.h"

enum {
	/* Timer repeat continuosly */
	TMR_RECURSIVE = (1 << 0),
	/* Timer is counting... */
	TMR_TRIGGERED = (1 << 1),
	/* Timer has expired */
	TMR_EXPIRED = (1 << 2),
};

struct timer {
	list_node header;
	uint64_t expiration;
	uint32_t flags;
	char name[16];
	unsigned msecs;
	tmr_cb cb;
	void *arg;
};

static list_inst timers_list;

static uint32_t period = -1U;

static barrier_t *bar = NULL;

static void timer_cb(uint64_t msecs)
{
	struct timer *cursor;
	unsigned expired = 0;
	period = -1U;

	cursor = list_head(&timers_list);

	while (cursor) {
		if (TMR_TRIGGERED == (cursor->flags & (TMR_TRIGGERED | TMR_EXPIRED))) {
			if (msecs >= cursor->expiration) {
				cursor->flags |= TMR_EXPIRED;
				expired++;
				/* One shot */
				if (!(cursor->flags & TMR_RECURSIVE)) {
					cursor->flags &= ~TMR_TRIGGERED;
				} else {
					if (cursor->msecs < period)
						period = cursor->msecs;
				}
			}
		}
		cursor = (timer_t *) cursor->header.next;
	}

	clock_set_period(period, CLK_INST_ALARMS);
	if (expired)
		barrier_open(bar);
}

BOOL init_timers_lib()
{
	if (EOK != list_init(&timers_list)) {
		kerrprintf("failed initing timer lib.\n");
		return (FALSE);
	}

	/*
	 * The timers API need a running thread, its entry point is
	 * timers_thread_entry.
	 * The kernel will take care of it as it is an internal reserved thread,
	 * just like the idle thread.
	 * This init function does not need to care about timers_thread_entry.
	 * In case the thread cannot be spawned, the kernel will fail and abort execution.
	 */
	return (TRUE);
}

timer_t *timer_create(const char *name, unsigned millisecs, BOOL recursive, tmr_cb cb, void *arg)
{
	struct timer *ptr;

	if (!millisecs || !cb) {
		errno = EINVAL;
		return (NULL);
	}

	ptr = malloc(sizeof(struct timer));

	if (!ptr) {
		errno = ENOMEM;
		return (NULL);
	}

	lock();

	if (list_count(&timers_list) == 0) {
		if (!clock_add_cb(timer_cb, CLK_INST_TIMERS)) {
			unlock();
			free(ptr);
			kerrprintf("failed adding timer callback!\n");
			errno = EPERM;
			return (NULL);
		}
	}

	if (EOK != list_prepend(&timers_list, &ptr->header)) {
		unlock();
		free(ptr);
		errno = EPERM;
		return (NULL);
	}

	if (name) {
		snprintf(ptr->name, sizeof(ptr->name), "%s", name);
	} else {
		snprintf(ptr->name, sizeof(ptr->name), "Timer%x", (intptr_t) ptr);
	}

	ptr->flags = 0;

	if (recursive) {
		ptr->flags |= TMR_RECURSIVE;
	}

	ptr->expiration = 0;
	ptr->msecs = millisecs;
	ptr->cb = cb;
	ptr->arg = arg;

	/* 
	 * Shorten clock expiration if the required timeout
	 * is lower than the last computed period.
	 * Do not update the computed period, that's up to the
	 * interrupt routine.
	 */
	if (millisecs < period)
		clock_set_period(millisecs, CLK_INST_TIMERS);

	unlock();

	return (ptr);
}

void timer_set(timer_t * tmr, BOOL start)
{
	if (!tmr) {
		errno = EINVAL;
		return;
	}

	lock();

	/*
	 * The routine need to clear or set TMR_EXPIRED
	 * to handle properly the invocation of the callback.
	 */
	if (start) {
		tmr->flags |= TMR_TRIGGERED;
		tmr->flags &= ~TMR_EXPIRED;
		tmr->expiration = tmr->msecs + clock_get_milliseconds();
	} else {
		tmr->flags &= ~TMR_TRIGGERED;
		tmr->flags |= TMR_EXPIRED;
		tmr->expiration = 0;
	}

	unlock();
}

int timer_update(timer_t * tmr, unsigned millisecs, BOOL recursive)
{
	if (!tmr || !millisecs) {
		return (EINVAL);
	}

	lock();

	if (millisecs != tmr->msecs) {
		tmr->msecs = millisecs;
		/* 
		 * Shorten clock expiration if the required timeout
		 * is lower than the last computed period.
		 * Do not update the computed period, that's up to the
		 * interrupt routine.
		 */
		if (millisecs < period)
			clock_set_period(millisecs, CLK_INST_TIMERS);
	}

	if (recursive && !(tmr->flags & TMR_RECURSIVE)) {
		tmr->flags |= TMR_RECURSIVE;
	} else if (!recursive && (tmr->flags & TMR_RECURSIVE)) {
		tmr->flags &= ~TMR_RECURSIVE;
	}

	unlock();

	return (EOK);
}

int timer_done(timer_t * tmr)
{
	if (!tmr) {
		return (EINVAL);
	}

	lock();

	if (EOK != list_remove(&timers_list, &tmr->header)) {
		unlock();
		return (EPERM);
	}

	if (list_count(&timers_list) == 0) {
		clock_set_period(-1U, CLK_INST_TIMERS);
		if (!clock_del_cb(timer_cb, CLK_INST_TIMERS))
			kerrprintf("failed removing timer callback!\n");
	}

	unlock();

	free(tmr);

	return (EOK);
}

static void dump_internal(const timer_t * tmr)
{
	char tempbuf[60];

	sprintf(tempbuf, "(%#x) ", tmr->flags);
	if (tmr->flags & TMR_EXPIRED) {
		strcat(tempbuf, "EXPIRED ");
	}
	if (tmr->flags & TMR_RECURSIVE) {
		strcat(tempbuf, "RECURSIVE ");
	}
	if (tmr->flags & TMR_TRIGGERED) {
		strcat(tempbuf, "TRIGGERED ");
	}
	if (!tmr->flags) {
		strcat(tempbuf, "DISABLED");
	}
	printf("%-15s | %6u | %10llu | %s\n", tmr->name, tmr->msecs, tmr->expiration,
		tempbuf);
}

void timers_dump(const timer_t * tmr)
{
	if (!tmr) {
		printf("\n--- TIMERS TABLE -------------------------------------\n\n");
	}
	printf("%-15s   %6s   %10s    %s\n", "TIMER NAME", "PERIOD", "EXPIRATION",
		"FLAGS");
	printf("______________________________________________________\n");
	if (tmr) {
		dump_internal(tmr);
	} else {
		tmr = list_head(&timers_list);
		while (tmr) {
			dump_internal(tmr);
			tmr = (timer_t *) tmr->header.next;
		}
	}
	printf("------------------------------------------------------\n\n");
}

void timers_thread_entry(void)
{
	struct timer *cursor;

	bar = barrier_create("Timers", TRUE);

	if (!bar)
		kernel_panic("cannot create a barrier for timers thread.");

	while (EOK == wait_for_barrier(bar)) {

		if (!list_count(&timers_list)) {
			continue;
		}

		lock();

		cursor = list_head(&timers_list);

		while (cursor) {
			if (cursor->flags & TMR_EXPIRED) {
				cursor->cb(cursor->arg);
				cursor->flags &= ~TMR_EXPIRED;
				if (cursor->flags & TMR_RECURSIVE)
					cursor->expiration =
					    cursor->msecs + clock_get_milliseconds();
			}
			cursor = (timer_t *) cursor->header.next;
		}

		unlock();
	}

	barrier_done(bar);

	thread_terminate();
}
