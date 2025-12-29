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

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <libs/chunks.h>
#include <libs/bitmaps.h>
#include <diegos/io_waits.h>
#include <diegos/interrupts.h>

#include "poll_private.h"
#include "io_waits_private.h"
#include "threads.h"
#include "kprintf.h"
#include "scheduler.h"
#include "platform_include.h"

struct wait_queue_int {
	list_node header;
	wait_queue_t *wq;
};

static list_inst wait_queues;
static chunks_pool_t *wait_queue_items;
static chunks_pool_t *wait_queue_int_items;

static long thread_ids[BITMAPLEN(DIEGOS_MAX_THREADS)];

int thread_io_wait_init(wait_queue_t *wq)
{
	struct wait_queue_int *temp;

	if (!wq) {
		return EINVAL;
	}

	temp = chunks_pool_malloc(wait_queue_int_items);
	if (!temp) {
		return EPERM;
	}
	temp->wq = wq;

	if (EOK != list_append(&wait_queues, &temp->header)) {
		chunks_pool_free(wait_queue_int_items, temp);
		return EPERM;
	}

	return EOK;
}

static int thread_io_wait_internal(wait_queue_t *wq, unsigned flags)
{
	struct wait_queue_item *temp;
	thread_t *prev, *next;
	int retcode = EOK;

	if (!wq) {
		return EINVAL;
	}

	lock();

	temp = chunks_pool_malloc(wait_queue_items);
	if (!temp) {
		unlock();
		return EPERM;
	}

	temp->flags = flags;
	if (IO_WAIT_DEFAULT == flags) {
		temp->tid = scheduler_running_tid();
	} else {
		chunks_pool_free(wait_queue_items, temp);
		unlock();
		return EINVAL;
	}

	retcode = list_append(wq, &temp->header);

	if (EOK != retcode) {
		list_remove(wq, &temp->header);
		chunks_pool_free(wait_queue_items, temp);
		unlock();
		return EPERM;
	}

	prev = scheduler_running_thread();

	if (!scheduler_wait_thread(THREAD_FLAG_WAIT_COMPLETION, 0)) {
		kerrprintf("TID %u Cannot wait for I/O\n", prev);
		list_remove(wq, &temp->header);
		chunks_pool_free(wait_queue_items, temp);
		unlock();
		return EPERM;
	}

	unlock();

	schedule_thread();

	next = scheduler_running_thread();

	switch_context(&prev->context, next->context);

	return EOK;
}

int thread_io_wait(wait_queue_t *wq)
{
	return (thread_io_wait_internal(wq, IO_WAIT_DEFAULT));
}

int thread_io_resume(wait_queue_t *wq)
{
	struct wait_queue_item *cursor;

	if (!wq) {
		return (EINVAL);
	}

	lock();
	while (list_count(wq)) {
		cursor = list_head(wq);
		if (IO_WAIT_POLL == cursor->flags) {
			(void)poll_wakeup(cursor);
		} else {
			bitmap_set(thread_ids, cursor->tid);
		}
		if (EOK != list_remove(wq, &cursor->header)) {
			break;
		}
		chunks_pool_free(wait_queue_items, cursor);
	}
	unlock();

	return EOK;
}

/*
 * Private section
 */

BOOL init_io_wait_lib()
{
	if (EOK != list_init(&wait_queues)) {
		return (FALSE);
	}

	wait_queue_int_items = chunks_pool_create("waitqueue", 0,
						  sizeof(struct wait_queue_int), 8, 8);
	if (!wait_queue_int_items) {
		return (FALSE);
	}

	wait_queue_items = chunks_pool_create("wqitems",
					      sizeof(struct wait_queue_item),
					      sizeof(struct wait_queue_item), 8, 8);

	if (!wait_queue_items) {
		chunks_pool_done(wait_queue_int_items);
		return (FALSE);
	}

	memset(thread_ids, 0, sizeof(thread_ids));

	return (TRUE);
}

struct wait_queue_item *io_wait_get_item()
{
	struct wait_queue_item *retcode;

	lock();
	retcode = chunks_pool_malloc(wait_queue_items);
	unlock();

	return (retcode);
}

void io_wait_put_item(struct wait_queue_item *item)
{
	lock();
	chunks_pool_free(wait_queue_items, item);
	unlock();
}

int io_wait_add(struct wait_queue_item *item, wait_queue_t *wq)
{
	int retcode = EINVAL;

	if (item && wq) {
		lock();
		retcode = list_append(wq, &item->header);
		unlock();
	}

	return (retcode);
}

int io_wait_remove(struct wait_queue_item *item, wait_queue_t *wq)
{
	int retcode = EINVAL;

	if (item && wq) {
		lock();
		retcode = list_remove(wq, &item->header);
		unlock();
	}

	return (retcode);
}

static void resumecb(long *bitmap, unsigned pos, void *param)
{
	if (!scheduler_resume_thread(THREAD_FLAG_WAIT_COMPLETION, (uint8_t) pos)) {
		kerrprintf("TID %u Cannot be resumed for I/O\n", pos);
	}
	bitmap_clear(bitmap, pos);
}

void resume_on_io_wait()
{
	lock();
	bitmap_for_each_set(thread_ids, BITMAPLEN(DIEGOS_MAX_THREADS), resumecb, NULL);
	unlock();
}

int cancel_io_waits(uint8_t tid)
{
	struct wait_queue_int *wq_int;
	struct wait_queue_item *wq_item;
	list_node *ptr, *temp;

	lock();
	ptr = list_head(&wait_queues);
	while (ptr) {
		wq_int = (struct wait_queue_int *)ptr;
		temp = list_head(wq_int->wq);
		while (temp) {
			wq_item = (struct wait_queue_item *)temp;
			if (wq_item->tid == tid) {
				list_remove(wq_int->wq, &wq_item->header);
				chunks_pool_free(wait_queue_items, wq_item);
				break;
			}
			temp = temp->next;
		}
		ptr = ptr->next;
	}
	bitmap_clear(thread_ids, tid);
	unlock();

	return EOK;
}
