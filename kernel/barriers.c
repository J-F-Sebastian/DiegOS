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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <libs/list.h>
#include <diegos/barriers.h>
#include <diegos/interrupts.h>
#include <libs/bitmaps.h>
#include <string.h>

#include "barriers_private.h"
#include "threads_data.h"
#include "scheduler.h"
#include "platform_include.h"
#include "kprintf.h"

enum {
	/* Barrier state, open or closed */
	BARRIER_OPEN = (1 << 0),
	/* Open barriers will be automatially closed in resume_on_barriers */
	BARRIER_AUTOCLOSE = (1 << 1)
};

struct barrier {
	list_node header;
	long thread_ids[BITMAPLEN(DIEGOS_MAX_THREADS)];
	char name[16];
	unsigned flags;
};

static list_inst barriers_list;

barrier_t *barrier_create(const char *name, BOOL autoclose)
{
	struct barrier *ptr = malloc(sizeof(struct barrier));

	if (!ptr) {
		return (NULL);
	}

	if (EOK != list_prepend(&barriers_list, &ptr->header)) {
		free(ptr);
		return (NULL);
	}

	memset(ptr->thread_ids, 0, sizeof(ptr->thread_ids));

	if (name) {
		snprintf(ptr->name, sizeof(ptr->name), "%s", name);
	} else {
		snprintf(ptr->name, sizeof(ptr->name), "Barrier%x", (intptr_t) ptr);
	}

	if (autoclose) {
		ptr->flags = BARRIER_AUTOCLOSE;
	} else {
		ptr->flags = 0;
	}

	return (ptr);
}

int barrier_done(barrier_t * barrier)
{
	int retval;

	if (!barrier)
		return (EINVAL);

	resume_on_barriers();

	retval = list_remove(&barriers_list, &barrier->header);
	if (retval != EOK)
		kerrprintf("Invalid barrier");
	else
		free(barrier);

	return retval;
}

int barrier_open(barrier_t * barrier)
{
	if (barrier) {
		barrier->flags |= BARRIER_OPEN;
	}

	return ((barrier) ? EOK : EINVAL);
}

int barrier_close(barrier_t * barrier)
{
	if (barrier) {
		barrier->flags &= ~BARRIER_OPEN;
	}

	return ((barrier) ? EOK : EINVAL);
}

int wait_for_barrier(barrier_t * barrier)
{
	thread_t *prev, *next;

	if (!barrier) {
		kerrprintf("Invalid barrier");
		return EINVAL;
	}

	if (barrier->flags & BARRIER_OPEN) {
		return EOK;
	}

	bitmap_set(barrier->thread_ids, scheduler_running_tid());

	prev = scheduler_running_thread();
	if (!scheduler_wait_thread(THREAD_FLAG_WAIT_BARRIER, 0)) {
		return EPERM;
	}
	schedule_thread();

	next = scheduler_running_thread();
	switch_context(&prev->context, next->context);

	return EOK;
}

BOOL init_barriers_lib()
{
	if (EOK != list_init(&barriers_list)) {
		return (FALSE);
	}

	return (TRUE);
}

static void resumecb(long *bitmap, unsigned pos, void *param)
{
	scheduler_resume_thread(THREAD_FLAG_WAIT_BARRIER, (uint8_t) pos);
	bitmap_clear(bitmap, pos);
}

void resume_on_barriers()
{
	struct barrier *cur = (struct barrier *)list_head(&barriers_list);

	while (cur) {
		if (cur->flags & BARRIER_OPEN) {
			bitmap_for_each_set(cur->thread_ids,
					    BITMAPLEN(DIEGOS_MAX_THREADS), resumecb, NULL);
			if (cur->flags & BARRIER_AUTOCLOSE) {
				cur->flags &= ~BARRIER_OPEN;
			}
		}
		cur = (barrier_t *) cur->header.next;
	}
}

static void dump_internal(const barrier_t * barrier)
{
	printf("%-15s | %5s | %4s | %s\n",
		barrier->name,
		(barrier->flags & BARRIER_OPEN) ? "OPEN" : "CLOSED",
		(barrier->flags & BARRIER_AUTOCLOSE) ? "YES" : "NO", "N/A");
}

void barrier_dump(const barrier_t * barrier)
{
	if (!barrier) {
		printf("\n--- BARRIERS TABLE ----------------------\n\n");
	}
	printf("%-15s   %5s   %4s   %s\n",
		"BARRIER NAME", "STATE", "AUTO", "WAITING THREADS");
	printf("________________________________________\n");
	if (barrier) {
		dump_internal(barrier);
	} else {
		barrier = list_head(&barriers_list);
		while (barrier) {
			dump_internal(barrier);
			barrier = (barrier_t *) barrier->header.next;
		}
	}
	printf("----------------------------------------\n\n");
}
